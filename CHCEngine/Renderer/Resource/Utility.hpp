#pragma once
#include "../ClassName.h"
#include "../D3D12Convert.h"
#include "../D3D12Utilities.hpp"
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Resource {
    ComPtr<GPUResource> createBuffer(ComPtr<Device> device, UINT64 size,
        HeapType heap_type,
        ResourceState initial_state) {
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
  desc.Flags = D3D12_RESOURCE_FLAG_NONE;
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

}
}  // namespace Renderer
}  // namespace CHCEngine