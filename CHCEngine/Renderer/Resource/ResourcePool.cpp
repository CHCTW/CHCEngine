#include "../ClassName.h"

#include "ResourcePool.h"

#include <magic_enum.hpp>

#include "../D3D12Convert.h"
#include "Utilities.hpp"

namespace CHCEngine {
namespace Renderer {
namespace Resource {
unsigned long long ResourcePool::getNextBufferId(BufferType type) {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  if (!buffer_id_count_.count(type))
    buffer_id_count_[type] = 0;
  return buffer_id_count_[type]++;
}
unsigned long long ResourcePool::getNextTextureId(TextureType type) {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  if (!texture_id_count_.count(type))
    texture_id_count_[type] = 0;
  return texture_id_count_[type]++;
}
ResourcePool::ResourcePool(ComPtr<Device> device,
                           std::shared_ptr<DescriptorHeap> srv_uav_cbv,
                           std::shared_ptr<DescriptorHeap> rtv,
                           std::shared_ptr<DescriptorHeap> dsv)
    : device_(device) {
  static_heaps_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV] = srv_uav_cbv;
  static_heaps_[DescriptorType::DESCRIPTOR_TYPE_RTV] = rtv;
  static_heaps_[DescriptorType::DESCRIPTOR_TYPE_DSV] = dsv;
}

std::shared_ptr<Buffer> ResourcePool::getVertexBuffer(
    unsigned int vertex_count,
    const std::vector<std::pair<std::string, DataFormat>> &attributes,
    ResourceState initial_state, ResourceUpdateType update_type) {
  unsigned int stride_size = 0;
  unsigned int byte_size = 0;
  std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>
      vertex_attributes;

  unsigned long long buffer_size = getVertexBufferAttributes(
      vertex_count, stride_size, attributes, vertex_attributes);

  auto id = getNextBufferId(BufferType::BUFFER_TYPE_VERTEX);
  std::string name =
      std::string(magic_enum::enum_name<BufferType::BUFFER_TYPE_VERTEX>()) +
      '_' + std::to_string(id);
  auto gpu_resource = createBuffer(device_, buffer_size,
                                   HeapType::HEAP_TYPE_DEFAULT, initial_state);
  NAME_D3D12_OBJECT_STRING(gpu_resource, name);
  ComPtr<GPUResource> upload_buffer;
  if (update_type == ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC) {
    upload_buffer =
        createBuffer(device_, buffer_size, HeapType::HEAP_TYPE_UPLOAD,
                     ResourceState::RESOURCE_STATE_GENERIC_READ);
    std::string temp = name;
    temp += "_upload";
    NAME_D3D12_OBJECT_STRING(upload_buffer, temp);
  }
  ResourceInformation res_info = {name, ResourceType::RESOURCE_TYPE_BUFFER,
                                  update_type};
  BufferInformation buffer_information = {
      BufferType::BUFFER_TYPE_VERTEX, BufferReadWrite::BUFFER_READ_WRITE_READ,
      buffer_size, stride_size, vertex_attributes};
  auto vertex_view = getVertrexBufferView(
      gpu_resource, static_cast<unsigned int>(buffer_size), stride_size);

  return std::make_shared<Buffer>(gpu_resource, upload_buffer, res_info,
                                  buffer_information, vertex_view);
}

std::shared_ptr<Buffer>
ResourcePool::getIndexBuffer(unsigned int index_count, IndexFormat index_format,
                             ResourceState initial_state,
                             ResourceUpdateType update_type) {
  auto id = getNextBufferId(BufferType::BUFFER_TYPE_INDEX);
  std::string name =
      std::string(magic_enum::enum_name<BufferType::BUFFER_TYPE_INDEX>()) +
      '_' + std::to_string(id);
  std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>
      vertex_attributes;
  DataFormat format = DataFormat::DATA_FORMAT_R32_UINT;
  unsigned index_size = 4;
  if (index_format == IndexFormat::INDEX_FORMAT_16_UINT) {
    format = DataFormat::DATA_FORMAT_R16_UINT;
    index_size = 2;
  }
  vertex_attributes["INDEX"] = std::make_pair(0, format);
  unsigned int buffer_size = index_size * index_count;
  auto gpu_resource = createBuffer(device_, buffer_size,
                                   HeapType::HEAP_TYPE_DEFAULT, initial_state);
  NAME_D3D12_OBJECT_STRING(gpu_resource, name);
  ComPtr<GPUResource> upload_buffer;
  if (update_type == ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC) {
    upload_buffer =
        createBuffer(device_, buffer_size, HeapType::HEAP_TYPE_UPLOAD,
                     ResourceState::RESOURCE_STATE_COPY_SOURCE);
    std::string temp = name;
    temp += "_upload";
    NAME_D3D12_OBJECT_STRING(upload_buffer, temp);
  }
  ResourceInformation res_info = {name, ResourceType::RESOURCE_TYPE_BUFFER,
                                  update_type};
  BufferInformation buffer_information = {
      BufferType::BUFFER_TYPE_INDEX, BufferReadWrite::BUFFER_READ_WRITE_READ,
      buffer_size, index_size, vertex_attributes};
  auto index_view = std::make_shared<IndexBufferView>();

  index_view->BufferLocation = gpu_resource->GetGPUVirtualAddress();
  index_view->SizeInBytes = buffer_size;
  index_view->Format = convertToDXGIFormat(index_format);

  return std::make_shared<Buffer>(gpu_resource, upload_buffer, res_info,
                                  buffer_information, index_view);
}

std::shared_ptr<Buffer> ResourcePool::getBuffer(
    unsigned int element_count, unsigned int element_byte_size,
    const std::vector<BufferUsage> &usages,
    const std::vector<std::pair<std::string, DataFormat>> &attributes,
    IndexFormat index_format, ResourceState initial_state,
    ResourceUpdateType update_type) {
  if (usages.size() == 0 && attributes.size() == 0 &&
      index_format == IndexFormat::INDEX_FORMAT_NONE) {
    throw std::exception(
        "No descritpor nor vertex or index buffer,invalid buffer create");
  }
  UINT64 size = static_cast<UINT64>(element_count) *
                static_cast<UINT64>(element_byte_size);
  bool append_counter = false;
  bool as_constant = false;
  unsigned long long alignment = 1;
  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
  for (auto &usage : usages) {
    if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_CBV) {
      if (append_counter) {
        throw std::exception(
            "Can't use constant buffer and counter buffer in the same buffer");
      }
      as_constant = true;
    }
    if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_UAV)
      flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (usage.uav_use_counter_) {
      if (as_constant) {
        throw std::exception(
            "Can't use constant buffer and counter buffer in the same buffer");
      }
      append_counter = true;
    }
    if (usage.is_raw_buffer_) {
      if (usage.start_index_ % 4 != 0) {
        throw std::exception("Raw buffer use R32Uint as element and the start "
                             "index need to be the multiply of 4");
      }
      if (size <= usage.start_index_ * 4) {
        throw std::exception(
            "Raw buffer use R32Uint as element and the start "
            "index is large then the total element count( count of R32)");
      }
    }
  }
  if (as_constant)
    alignment = constant_buffer_alignment_;
  if (append_counter)
    alignment = counter_buffer_alignment_;
  size = (size + (alignment - 1)) & ~(alignment - 1);
  if (append_counter)
    size += count_size_;
  auto id = getNextBufferId(BufferType::BUFFER_TYPE_CUSTOM);
  std::string name =
      std::string(magic_enum::enum_name<BufferType::BUFFER_TYPE_CUSTOM>()) +
      '_' + std::to_string(id);
  auto gpu_resource = createBuffer(device_, size, HeapType::HEAP_TYPE_DEFAULT,
                                   initial_state, flags);
  NAME_D3D12_OBJECT_STRING(gpu_resource, name);
  ComPtr<GPUResource> upload_buffer;
  if (update_type == ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC) {
    upload_buffer = createBuffer(device_, size, HeapType::HEAP_TYPE_UPLOAD,
                                 ResourceState::RESOURCE_STATE_COPY_SOURCE);
    std::string temp = name;
    temp += "_upload";
    NAME_D3D12_OBJECT_STRING(upload_buffer, temp);
  }

  std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
      descriptor_ranges;
  if (usages.size() > 0) {

    descriptor_ranges[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV] =
        getBufferDesciptorRanges(
            device_, usages, gpu_resource, element_count, element_byte_size,
            size, static_heaps_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]);
  }
  std::shared_ptr<VertexBufferView> vertex_view;
  std::shared_ptr<IndexBufferView> index_view;
  std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>
      vertex_attributes;
  unsigned int stride_size = 0;
  if (attributes.size() != 0) {
    UINT64 ori_size = getVertexBufferAttributes(element_count, stride_size,
                                                attributes, vertex_attributes);
    if (stride_size != element_byte_size) {
      throw std::exception(
          "Vertex attributes stride size should be the same element byte size");
    }
    vertex_view = getVertrexBufferView(
        gpu_resource, static_cast<unsigned int>(ori_size), element_byte_size);
  }
  if (index_format != IndexFormat::INDEX_FORMAT_NONE) {
    stride_size = 4;
    DataFormat format = DataFormat::DATA_FORMAT_R32_UINT;
    if (index_format == IndexFormat::INDEX_FORMAT_16_UINT) {
      format = DataFormat::DATA_FORMAT_R16_UINT;
      stride_size = 2;
    }
    vertex_attributes["INDEX"] = std::make_pair(0, format);
    index_view = std::make_shared<IndexBufferView>();
    index_view->BufferLocation = gpu_resource->GetGPUVirtualAddress();
    index_view->SizeInBytes =
        static_cast<unsigned int>(static_cast<UINT64>(element_count) *
                                  static_cast<UINT64>(element_byte_size));
    index_view->Format = convertToDXGIFormat(index_format);
  }
  ResourceInformation res_info = {name, ResourceType::RESOURCE_TYPE_BUFFER,
                                  update_type};
  BufferInformation buffer_information = {
      BufferType::BUFFER_TYPE_CUSTOM, BufferReadWrite::BUFFER_READ_WRITE_READ,
      size, element_count, vertex_attributes};

  return std::make_shared<Buffer>(gpu_resource, upload_buffer, res_info,
                                  buffer_information, descriptor_ranges,
                                   vertex_view, index_view);
}

std::shared_ptr<Texture> ResourcePool::getTexture(
    TextureType texture_type, RawFormat raw_format, unsigned long long width,
    unsigned int height, unsigned int depth, unsigned int mip_levels,
    const std::vector<TextureUsage> &usages,
    const std::vector<RenderTargetUsage> &render_target_usages,
    const std::vector<DepthStencilUsage> &depth_stencil_usages,
    ResourceState initial_state,
    ResourceUpdateType update_type) {
  if (usages.empty()&&render_target_usages.empty()&&depth_stencil_usages.empty()) {
    throw std::exception("No usages, invalid texture create");
  }
  std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
      descriptor_ranges;
  bool use_shader_resource_view = false;
  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
  for (unsigned int i = 0; i < usages.size(); ++i) {
    if (usages[i].usage_ == ResourceUsage::RESOURCE_USAGE_UAV) {
      flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    } else if (usages[i].usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
      use_shader_resource_view = true;
    }
    else {
      throw std::exception(
          (std::string("Error usage with texture : ") +
           std::string(magic_enum::enum_name(usages[i].usage_)))
              .c_str());
    }
  }
  if (render_target_usages.size()) {
    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  }
  if (depth_stencil_usages.size()) {
    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  }
  if ((flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) &&
      ((flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) |
       (flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))) {
    throw std::exception("Can't use depth stencil with rener target or unordered access together");
  }
  if (!use_shader_resource_view &&
      (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
    flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
  ComPtr<GPUResource> upload_buffer = nullptr;
  ComPtr<GPUResource> gpu_resource =
      createTexture(device_, convertToD3D12ResourceDimension(texture_type),
                    convertToDXGIFormat(convertToDataFormat(raw_format)), width, height, depth,
                    HeapType::HEAP_TYPE_DEFAULT, mip_levels, initial_state,flags);

  if (usages.size()) {
    descriptor_ranges[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV] =
        getSRVUAVTextureDesciptorRanges(
            device_, usages, gpu_resource, texture_type, depth,
            static_heaps_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]);
  }
  if (render_target_usages.size()) {
    descriptor_ranges[DescriptorType::DESCRIPTOR_TYPE_RTV] =
        getRTVTextureDesciptorRanges(
            device_, render_target_usages, gpu_resource, texture_type, depth,
            static_heaps_[DescriptorType::DESCRIPTOR_TYPE_RTV]);
  }
  if (depth_stencil_usages.size()) {
    descriptor_ranges[DescriptorType::DESCRIPTOR_TYPE_DSV] =
        getDSVTextureDesciptorRanges(
            device_, depth_stencil_usages, gpu_resource, texture_type, depth,
            static_heaps_[DescriptorType::DESCRIPTOR_TYPE_DSV]);
  }
  unsigned long long id = getNextTextureId(texture_type);
  std::string name(magic_enum::enum_name(texture_type));
  name += "_" + std::to_string(id);
  NAME_D3D12_OBJECT_STRING(gpu_resource, name);

  unsigned int subresouce_count = depth * mip_levels;
  if (texture_type == TextureType::TEXTURE_TYPE_3D)
    subresouce_count = mip_levels;
  D3D12_RESOURCE_DESC desc = gpu_resource->GetDesc();
  UINT64 required_size = 0;
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT *foot_prints =
      new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[subresouce_count];
  unsigned int *row_counts = new unsigned int[subresouce_count];
  unsigned long long *row_byte_size =
      new unsigned long long[subresouce_count];
  device_->GetCopyableFootprints(&desc, 0, subresouce_count, 0, foot_prints,
                                 row_counts, row_byte_size, &required_size);
  std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> foot_print_vec(
      foot_prints, foot_prints+subresouce_count);
  std::vector<unsigned int> row_counts_vec(row_counts,
                                           row_counts+subresouce_count);
  std::vector<unsigned long long> row_bytes_vec(row_byte_size,
                                                row_byte_size+subresouce_count);
  delete[] foot_prints;
  delete[] row_counts;
  delete[] row_byte_size;
  ResourceInformation information = {name, ResourceType::RESOURCE_TYPE_TEXTURE,
                                     update_type};
  if (update_type == ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC) {
    upload_buffer =
        createBuffer(device_, required_size, HeapType::HEAP_TYPE_UPLOAD,
                     ResourceState::RESOURCE_STATE_COPY_SOURCE);
    name += "_upload";
    NAME_D3D12_OBJECT_STRING(gpu_resource, name);
  }
  TextureInformation text_inf = {texture_type, raw_format, width,
                                 height,
                                 depth,
                                 mip_levels,
                                 std::move(foot_print_vec),
                                 std::move(row_counts_vec),
                                 std::move(row_bytes_vec),
                                 required_size};
  return std::make_shared<Texture>(gpu_resource, upload_buffer, information,
                                   text_inf, descriptor_ranges);
}

} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine