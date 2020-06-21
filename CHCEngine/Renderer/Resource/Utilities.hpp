#pragma once
#include "../ClassName.h"
#include "../D3D12Convert.h"
#include "../D3D12Utilities.hpp"
#include "../DescriptorHeap.h"
//#include "../Re"

#include <unordered_map>
#include <unordered_set>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Resource {
ComPtr<GPUResource> createBuffer(ComPtr<Device> device, UINT64 size,
                                 HeapType heap_type,
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
std::shared_ptr<DescriptorRange> getBufferDesciptorRanges(
    ComPtr<Device> device, const std::vector<BufferUsage> &usages,
    ComPtr<GPUResource> gpu_resource, unsigned int element_count,
    unsigned int element_byte_size, unsigned long long size,
    std::shared_ptr<DescriptorHeap> srv_uav_cbv) {
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
      ComPtr<GPUResource> counter_res = nullptr;
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
          (p.first + "have the same sematic name in one vertex attributes")
              .c_str());
    }
    vetex_attributes[p.first] = std::make_pair(stride_size, p.second);
    stride_size += byte_size;
  }
  return stride_size;
}

unsigned long long getVertexBufferAttributes(
    unsigned int vertex_count,
    unsigned int &stride_size,
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
          (p.first + "have the same sematic name in one vertex attributes")
              .c_str());
    }
    vetex_attributes[p.first] = std::make_pair(stride_size, p.second);
    stride_size += byte_size;
  }
  unsigned long long buffer_size = static_cast<UINT64>(stride_size) * static_cast<UINT64>(vertex_count);
  return buffer_size;
}
std::shared_ptr<VertexBufferView>
getVertrexBufferView(ComPtr<GPUResource> gpu_resource, unsigned int buffer_size, unsigned int stride_size) {
  auto vertex_view = std::make_shared<VertexBufferView>();

  vertex_view->BufferLocation = gpu_resource->GetGPUVirtualAddress();
  vertex_view->SizeInBytes = buffer_size;
  vertex_view->StrideInBytes = stride_size;
  return vertex_view;
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine