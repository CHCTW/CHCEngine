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
      std::string(magic_enum::enum_name<
                  BufferType::BUFFER_TYPE_VERTEX>()) +'_'+
                     std::to_string(id);
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
      std::string(magic_enum::enum_name<
                  BufferType::BUFFER_TYPE_INDEX>()) +'_'+
      std::to_string(id);
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
        throw std::exception("Can't use constant buffer and counter buffer in the same buffer");
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
      if (usage.start_index_%4!=0) {
        throw std::exception(
            "Raw buffer use R32Uint as element and the start index need to be the multiply of 4");
      }
      if (size <= usage.start_index_*4) {
        throw std::exception("Raw buffer use R32Uint as element and the start "
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
      std::string(magic_enum::enum_name<
                  BufferType::BUFFER_TYPE_CUSTOM>()) +'_'+
      std::to_string(id);
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
  std::vector<std::pair<DescriptorType, unsigned int>> usage_indices;
  if (usages.size() > 0) {

    descriptor_ranges[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV] =
        getBufferDesciptorRanges(
        device_, usages, gpu_resource, element_count, element_byte_size, size,
        static_heaps_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]);
    for (int i = 0; i < usages.size(); ++i) {
      usage_indices.push_back(
          {DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV, i});
    }
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
                                  usage_indices, vertex_view, index_view);
}

} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine