#pragma once
#include "../ClassName.h"

#include "../D3D12Convert.h"
#include "../D3D12Utilities.hpp"
#include "../DescriptorHeap.h"
//#include "../Re"

#include <limits>
#include <magic_enum.hpp>
#include <unordered_map>
#include <unordered_set>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Resource {
ComPtr<GPUResource>
createBuffer(const ComPtr<Device> &device, UINT64 size, HeapType heap_type,
             ResourceState initial_state,
             D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) {
  ComPtr<GPUResource> res;
  D3D12_HEAP_PROPERTIES heap_property;
  heap_property.Type = convertToD3D12HeapType(heap_type);
  heap_property.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_property.CreationNodeMask = 1;
  heap_property.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_property.VisibleNodeMask = 1;

  D3D12_RESOURCE_DESC desc;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Width = size;
  desc.Alignment = 0;
  desc.Flags = flags;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  ThrowIfFailed(device->CreateCommittedResource(
      &heap_property, D3D12_HEAP_FLAG_NONE, &desc,
      convertToD3D12ResourceStates(initial_state), nullptr,
      IID_PPV_ARGS(&res)));
  return res;
}
ComPtr<GPUResource>
createTexture(const ComPtr<Device> &device, D3D12_RESOURCE_DIMENSION dimension,
              DXGI_FORMAT format, UINT64 width, UINT height, UINT depth,
              HeapType heap_type, UINT mip_levels,
              D3D12_CLEAR_VALUE *clear_value, ResourceState initial_state,
              D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
              UINT sample_count = 1, UINT sample_quality = 0) {
  ComPtr<GPUResource> res;
  D3D12_HEAP_PROPERTIES heap_property;
  heap_property.Type = convertToD3D12HeapType(heap_type);
  heap_property.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_property.CreationNodeMask = 1;
  heap_property.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_property.VisibleNodeMask = 1;

  D3D12_RESOURCE_DESC desc;
  desc.Format = format;
  desc.Dimension = dimension;
  desc.Width = width;
  desc.Alignment = 0;
  desc.Flags = flags;
  desc.Height = height;
  desc.DepthOrArraySize = depth;
  desc.MipLevels = mip_levels;
  desc.SampleDesc.Count = sample_count;
  desc.SampleDesc.Quality = sample_quality;
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

  ThrowIfFailed(device->CreateCommittedResource(
      &heap_property, D3D12_HEAP_FLAG_NONE, &desc,
      convertToD3D12ResourceStates(initial_state), clear_value,
      IID_PPV_ARGS(&res)));
  return res;
}
std::shared_ptr<DescriptorRange> getBufferDesciptorRanges(
    const ComPtr<Device> &device, const std::vector<BufferUsage> &usages,
    const ComPtr<GPUResource> &gpu_resource, unsigned int element_count,
    unsigned int element_byte_size, unsigned long long size,
    const std::shared_ptr<DescriptorHeap> &srv_uav_cbv) {
  for (auto &usage : usages) {
    DescriptorType type = getDescriptorType(usage.usage_);
    if (type != DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV) {
      throw std::exception("Buffer and only be uav,srv and csv usage");
    }
  }
  std::shared_ptr<DescriptorRange> range =
      srv_uav_cbv->allocateRange(static_cast<unsigned int>(usages.size()));
  for (unsigned int i = 0; i < usages.size(); ++i) {
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
    ComPtr<GPUResource> counter_res = nullptr;
    switch (usages[i].usage_) {
    case ResourceUsage::RESOURCE_USAGE_CBV:
      cbv_desc.BufferLocation = gpu_resource->GetGPUVirtualAddress();
      cbv_desc.SizeInBytes = static_cast<unsigned int>(size);
      device->CreateConstantBufferView(&cbv_desc, range->getHandle(i));
      break;
    case ResourceUsage::RESOURCE_USAGE_SRV:
      srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      srv_desc.Format = DXGI_FORMAT_UNKNOWN;
      srv_desc.Shader4ComponentMapping =
          D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srv_desc.Buffer.FirstElement = usages[i].start_index_;
      srv_desc.Buffer.NumElements = element_count;
      srv_desc.Buffer.StructureByteStride = element_byte_size;
      if (usages[i].is_raw_buffer_) {
        srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
        srv_desc.Buffer.StructureByteStride = 0;
        srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
      }
      device->CreateShaderResourceView(gpu_resource.Get(), &srv_desc,
                                       range->getHandle(i));
      break;
    case ResourceUsage::RESOURCE_USAGE_UAV:
      uav_desc.Format = DXGI_FORMAT_UNKNOWN;
      uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
      uav_desc.Buffer.FirstElement = usages[i].start_index_;
      uav_desc.Buffer.NumElements = element_count;
      uav_desc.Buffer.StructureByteStride = element_byte_size;
      if (usages[i].uav_use_counter_) {
        uav_desc.Buffer.CounterOffsetInBytes = size - count_size_;
        counter_res = gpu_resource;
      }
      if (usages[i].is_raw_buffer_) {
        uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
        uav_desc.Buffer.StructureByteStride = 0;
        uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
      }
      device->CreateUnorderedAccessView(gpu_resource.Get(), counter_res.Get(),
                                        &uav_desc, range->getHandle(i));
      break;
    default:
      throw std::exception(
          (std::string("Error usage with buffer : ") +
           std::string(magic_enum::enum_name(usages[i].usage_)))
              .c_str());
      break;
    }
  }
  return range;
}
unsigned int getStrideSize(
    const std::vector<std::pair<std::string, DataFormat>> &attributes,
    std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>
        &vetex_attributes) {
  unsigned int stride_size = 0;
  unsigned int byte_size = 0;
  std::unordered_set<std::string> sematics;
  for (auto &p : attributes) {
    byte_size = getDataFormatByteSize(p.second);
    if (byte_size == 0) {
      throw std::exception((std::string(magic_enum::enum_name(p.second)) +
                            "  is not a valid format for vertex attributes")
                               .c_str());
    }
    if (vetex_attributes.count(p.first)) {
      throw std::exception(
          (p.first + "have the same semantic name in one vertex attributes")
              .c_str());
    }
    vetex_attributes[p.first] = std::make_pair(stride_size, p.second);
    stride_size += byte_size;
  }
  return stride_size;
}

unsigned long long getVertexBufferAttributes(
    unsigned int vertex_count, unsigned int &stride_size,
    const std::vector<std::pair<std::string, DataFormat>> &attributes,
    std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>
        &vetex_attributes) {
  unsigned int byte_size = 0;
  for (auto &p : attributes) {
    byte_size = getDataFormatByteSize(p.second);
    if (byte_size == 0) {
      throw std::exception((std::string(magic_enum::enum_name(p.second)) +
                            "  is not a valid format for vertex attributes")
                               .c_str());
    }
    if (vetex_attributes.count(p.first)) {
      throw std::exception(
          (p.first + "have the same semantic name in one vertex attributes")
              .c_str());
    }
    vetex_attributes[p.first] = std::make_pair(stride_size, p.second);
    stride_size += byte_size;
  }
  unsigned long long buffer_size =
      static_cast<UINT64>(stride_size) * static_cast<UINT64>(vertex_count);
  return buffer_size;
}
std::shared_ptr<VertexBufferView>
getVertrexBufferView(ComPtr<GPUResource> gpu_resource, unsigned int buffer_size,
                     unsigned int stride_size) {
  auto vertex_view = std::make_shared<VertexBufferView>();

  vertex_view->BufferLocation = gpu_resource->GetGPUVirtualAddress();
  vertex_view->SizeInBytes = buffer_size;
  vertex_view->StrideInBytes = stride_size;
  return vertex_view;
}
// only work on srv usage vector, should have none other usage type
std::shared_ptr<DescriptorRange> getSRVUAVTextureDesciptorRanges(
    const ComPtr<Device> &device, const std::vector<TextureUsage> &usages,
    const ComPtr<GPUResource> &gpu_resource, TextureType texture_type,
    unsigned int depth,
    const std::shared_ptr<DescriptorHeap> &descriptor_heap) {
  std::shared_ptr<DescriptorRange> range =
      descriptor_heap->allocateRange(static_cast<unsigned int>(usages.size()));
  unsigned int index = 0;
  unsigned int max_cubes = 0;
  for (auto &usage : usages) {
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
    if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
      srv_desc.Format = convertToDXGIFormat(usage.data_format_);
      srv_desc.Shader4ComponentMapping =
          D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    } else {
      uav_desc.Format = convertToDXGIFormat(usage.data_format_);
    }
    DataDimension dimension = usage.data_dimension_;
    if (dimension == DataDimension::DATA_DIMENSION_UNKNOWN)
      dimension = getDataDimension(texture_type, depth);
    switch (dimension) {
    case DataDimension::DATA_DIMENSION_TEXTURE1D:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        srv_desc.Texture1D.MostDetailedMip = usage.mip_range_.mips_start_level_;
        srv_desc.Texture1D.MipLevels = usage.mip_range_.mips_count_;
        srv_desc.Texture1D.ResourceMinLODClamp =
            usage.mip_range_.lod_lower_bound_;
      } else {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
        uav_desc.Texture1D.MipSlice = usage.mip_slice_;
      }
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE1DARRAY:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        srv_desc.Texture1DArray.MostDetailedMip =
            usage.mip_range_.mips_start_level_;
        srv_desc.Texture1DArray.MipLevels = usage.mip_range_.mips_count_;
        srv_desc.Texture1DArray.ResourceMinLODClamp =
            usage.mip_range_.lod_lower_bound_;
        srv_desc.Texture1DArray.FirstArraySlice =
            usage.sub_texture_ranges_.array_start_index_;
        srv_desc.Texture1DArray.ArraySize =
            (std::min)(usage.sub_texture_ranges_.count_, depth);
      } else {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        uav_desc.Texture1DArray.MipSlice = usage.mip_slice_;
        uav_desc.Texture1DArray.FirstArraySlice =
            usage.sub_texture_ranges_.array_start_index_;
        uav_desc.Texture1DArray.ArraySize =
            (std::min)(usage.sub_texture_ranges_.count_, depth);
      }
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2D:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MostDetailedMip = usage.mip_range_.mips_start_level_;
        srv_desc.Texture2D.MipLevels = usage.mip_range_.mips_count_;
        srv_desc.Texture2D.ResourceMinLODClamp =
            usage.mip_range_.lod_lower_bound_;
        srv_desc.Texture2D.PlaneSlice = usage.plane_slice_;
      } else {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = usage.mip_slice_;
        uav_desc.Texture2D.PlaneSlice = usage.plane_slice_;
      }
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DARRAY:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srv_desc.Texture2DArray.MostDetailedMip =
            usage.mip_range_.mips_start_level_;
        srv_desc.Texture2DArray.MipLevels = usage.mip_range_.mips_count_;
        srv_desc.Texture2DArray.ResourceMinLODClamp =
            usage.mip_range_.lod_lower_bound_;
        srv_desc.Texture2DArray.ArraySize =
            (std::min)(usage.sub_texture_ranges_.count_, depth);
        srv_desc.Texture2DArray.FirstArraySlice =
            usage.sub_texture_ranges_.array_start_index_;
      } else {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uav_desc.Texture2DArray.MipSlice = usage.mip_slice_;
        uav_desc.Texture2DArray.PlaneSlice = usage.plane_slice_;
        uav_desc.Texture2DArray.ArraySize =
            (std::min)(usage.sub_texture_ranges_.count_, depth);
        uav_desc.Texture2DArray.FirstArraySlice =
            usage.sub_texture_ranges_.array_start_index_;
      }
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DMS:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
      } else {
        throw std::exception(
            (std::string(
                 "Wrong dimension for texture unordered access view  : ") +
             std::string(magic_enum::enum_name(usage.data_dimension_)))
                .c_str());
      }
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DMSARRAY:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        srv_desc.Texture2DMSArray.ArraySize =
            (std::min)(usage.sub_texture_ranges_.count_, depth);
        srv_desc.Texture2DMSArray.FirstArraySlice =
            usage.sub_texture_ranges_.array_start_index_;
      } else {
        throw std::exception(
            (std::string(
                 "Wrong dimension for texture unordered access view  : ") +
             std::string(magic_enum::enum_name(usage.data_dimension_)))
                .c_str());
      }
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE3D:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srv_desc.Texture3D.MostDetailedMip = usage.mip_range_.mips_start_level_;
        srv_desc.Texture3D.MipLevels = usage.mip_range_.mips_count_;
        srv_desc.Texture3D.ResourceMinLODClamp =
            usage.mip_range_.lod_lower_bound_;
      } else {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uav_desc.Texture3D.MipSlice = usage.mip_slice_;
        uav_desc.Texture3D.FirstWSlice =
            usage.sub_texture_ranges_.array_start_index_;
        uav_desc.Texture3D.WSize =
            (std::min)(usage.sub_texture_ranges_.count_, depth);
      }
      break;
    case DataDimension::DATA_DIMENSION_TEXTURECUBE:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srv_desc.TextureCube.MostDetailedMip =
            usage.mip_range_.mips_start_level_;
        srv_desc.TextureCube.MipLevels = usage.mip_range_.mips_count_;
        srv_desc.TextureCube.ResourceMinLODClamp =
            usage.mip_range_.lod_lower_bound_;
        max_cubes = depth / 6;
        if (max_cubes < 1) {
          throw std::exception(
              (std::string("Depth of the 2d texture is too small to generate a "
                           "cube, with depth : ") +
               std::to_string(depth))
                  .c_str());
        }
      } else {
        throw std::exception(
            (std::string(
                 "Wrong dimension for texture unordered access view  : ") +
             std::string(magic_enum::enum_name(usage.data_dimension_)))
                .c_str());
      }
      break;
    case DataDimension::DATA_DIMENSION_TEXTURECUBEARRAY:
      if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        srv_desc.TextureCubeArray.MostDetailedMip =
            usage.mip_range_.mips_start_level_;
        srv_desc.TextureCubeArray.MipLevels = usage.mip_range_.mips_count_;
        srv_desc.TextureCubeArray.ResourceMinLODClamp =
            usage.mip_range_.lod_lower_bound_;
        srv_desc.TextureCubeArray.First2DArrayFace =
            usage.sub_texture_ranges_.array_start_index_;
        max_cubes = (depth - usage.sub_texture_ranges_.array_start_index_) / 6;
        if (max_cubes < 1) {
          throw std::exception(
              (std::string("Depth of the 2d texture is too small to generate "
                           "cubes, with start index : ") +
               std::to_string(usage.sub_texture_ranges_.array_start_index_) +
               " with depth :" + std::to_string(depth))
                  .c_str());
        }
        if (usage.sub_texture_ranges_.count_ ==
            (std::numeric_limits<unsigned int>::max)()) {
          srv_desc.TextureCubeArray.NumCubes = max_cubes;
        } else {
          if (usage.sub_texture_ranges_.count_ > max_cubes) {
            throw std::exception(
                (std::string(
                     "The cube counts is larger than maximum cubes in the "
                     "texture 2d, with count of cubes : ") +
                 std::to_string(usage.sub_texture_ranges_.count_))
                    .c_str());
          }
          srv_desc.TextureCubeArray.NumCubes = usage.sub_texture_ranges_.count_;
        }
      } else {
        throw std::exception(
            (std::string(
                 "Wrong dimension for texture unordered access view  : ") +
             std::string(magic_enum::enum_name(usage.data_dimension_)))
                .c_str());
      }
      break;
    default:
      throw std::exception(
          (std::string("Wrong dimension with texture shader resource view and "
                       "unorder access view : ") +
           std::string(magic_enum::enum_name(usage.data_dimension_)))
              .c_str());
    }
    if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
      device->CreateShaderResourceView(gpu_resource.Get(), &srv_desc,
                                       range->getHandle(index));
    } else {
      device->CreateUnorderedAccessView(gpu_resource.Get(), nullptr, &uav_desc,
                                        range->getHandle(index));
    }
    ++index;
  }
  return range;
}
std::shared_ptr<DescriptorRange> getRTVTextureDesciptorRanges(
    const ComPtr<Device> &device, const std::vector<RenderTargetUsage> &usages,
    const ComPtr<GPUResource> &gpu_resource, TextureType texture_type,
    unsigned int depth,
    const std::shared_ptr<DescriptorHeap> &descriptor_heap) {
  std::shared_ptr<DescriptorRange> range =
      descriptor_heap->allocateRange(static_cast<unsigned int>(usages.size()));
  unsigned int index = 0;
  for (auto &usage : usages) {
    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format = convertToDXGIFormat(usage.data_format_);
    DataDimension dimension = usage.data_dimension_;
    if (dimension == DataDimension::DATA_DIMENSION_UNKNOWN)
      dimension = getDataDimension(texture_type, depth);
    switch (dimension) {
    case DataDimension::DATA_DIMENSION_TEXTURE1D:
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
      rtv_desc.Texture1D.MipSlice = usage.mip_slice_;
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE1DARRAY:
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
      rtv_desc.Texture1DArray.MipSlice = usage.mip_slice_;
      rtv_desc.Texture1DArray.FirstArraySlice =
          usage.sub_texture_ranges_.array_start_index_;
      rtv_desc.Texture1DArray.ArraySize =
          (std::min)(usage.sub_texture_ranges_.count_, depth);
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2D:
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
      rtv_desc.Texture2D.MipSlice = usage.mip_slice_;
      rtv_desc.Texture2D.PlaneSlice = usage.plane_slice_;
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DARRAY:
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      rtv_desc.Texture2DArray.MipSlice = usage.mip_slice_;
      rtv_desc.Texture2DArray.PlaneSlice = usage.plane_slice_;
      rtv_desc.Texture2DArray.FirstArraySlice =
          usage.sub_texture_ranges_.array_start_index_;
      rtv_desc.Texture2DArray.ArraySize =
          (std::min)(usage.sub_texture_ranges_.count_, depth);
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DMS:
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DMSARRAY:
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
      rtv_desc.Texture2DMSArray.FirstArraySlice =
          usage.sub_texture_ranges_.array_start_index_;
      rtv_desc.Texture2DMSArray.ArraySize =
          (std::min)(usage.sub_texture_ranges_.count_, depth);
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE3D:
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
      rtv_desc.Texture3D.MipSlice = usage.mip_slice_;
      rtv_desc.Texture3D.FirstWSlice =
          usage.sub_texture_ranges_.array_start_index_;
      rtv_desc.Texture3D.WSize = usage.sub_texture_ranges_.count_;
      break;
    default:
      throw std::exception(
          (std::string("Wrong dimension with render target view : ") +
           std::string(magic_enum::enum_name(usage.data_dimension_)))
              .c_str());
    }
    device->CreateRenderTargetView(gpu_resource.Get(), &rtv_desc,
                                   range->getHandle(index));
    ++index;
  }
  return range;
}
std::shared_ptr<DescriptorRange> getDSVTextureDesciptorRanges(
    const ComPtr<Device> &device, const std::vector<DepthStencilUsage> &usages,
    const ComPtr<GPUResource> &gpu_resource, TextureType texture_type,
    unsigned int depth,
    const std::shared_ptr<DescriptorHeap> &descriptor_heap) {
  std::shared_ptr<DescriptorRange> range =
      descriptor_heap->allocateRange(static_cast<unsigned int>(usages.size()));
  unsigned int index = 0;
  for (auto &usage : usages) {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};

    dsv_desc.Format =
        convertToDXGIFormat(convertToDataFormat(usage.data_format_));
    dsv_desc.Flags = convertToD3D12DSVFlags(usage.depth_stencil_flag_);
    DataDimension dimension = usage.data_dimension_;
    if (dimension == DataDimension::DATA_DIMENSION_UNKNOWN)
      dimension = getDataDimension(texture_type, depth);
    switch (dimension) {
    case DataDimension::DATA_DIMENSION_TEXTURE1D:
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
      dsv_desc.Texture1D.MipSlice = usage.mip_slice_;
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE1DARRAY:
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
      dsv_desc.Texture1DArray.MipSlice = usage.mip_slice_;
      dsv_desc.Texture1DArray.FirstArraySlice =
          usage.sub_texture_ranges_.array_start_index_;
      dsv_desc.Texture1DArray.ArraySize =
          (std::min)(usage.sub_texture_ranges_.count_, depth);
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2D:
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsv_desc.Texture2D.MipSlice = usage.mip_slice_;
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DARRAY:
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
      dsv_desc.Texture2DArray.FirstArraySlice =
          usage.sub_texture_ranges_.array_start_index_;
      dsv_desc.Texture2DArray.ArraySize =
          (std::min)(usage.sub_texture_ranges_.count_, depth);
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DMS:
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
      break;
    case DataDimension::DATA_DIMENSION_TEXTURE2DMSARRAY:
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
      dsv_desc.Texture2DMSArray.FirstArraySlice =
          usage.sub_texture_ranges_.array_start_index_;
      dsv_desc.Texture2DMSArray.ArraySize =
          (std::min)(usage.sub_texture_ranges_.count_, depth);
      break;
    }
    device->CreateDepthStencilView(gpu_resource.Get(), &dsv_desc,
                                   range->getHandle(index));
    ++index;
  }
  return range;
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine