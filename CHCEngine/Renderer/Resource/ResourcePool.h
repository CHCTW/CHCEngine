#pragma once

#include <wrl/client.h>

#include <unordered_map>
#include <mutex>
#include "../DescriptorHeap.h"
#include "Buffer.h"

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Resource {
// the part really create the buffe and texture
// these give the free if I want to make some memeory management here
class ResourcePool {
 private:
  ComPtr<Device> device_;
  std::unordered_map<DescriptorType, std::shared_ptr<DescriptorHeap>>
      static_heaps_;
  /*ComPtr<GPUResource> createBuffer(UINT64 size, HeapType heap_type,
                                   ResourceState initial_state);*/
  std::unordered_map<BufferType, unsigned long long> buffer_id_count_;
  std::mutex pool_mutex_;
  unsigned long long getNextBufferId(BufferType type);

 public:
  ResourcePool(ComPtr<Device> device,
               std::shared_ptr<DescriptorHeap> srv_uav_cbv,
               std::shared_ptr<DescriptorHeap> rtv,
               std::shared_ptr<DescriptorHeap> dsv);
  // we assume the vertex buffer is wait for upload
  std::shared_ptr<Buffer> getVertexBuffer(
      unsigned int vertex_count,
      const std::vector<std::pair<std::string, DataFormat>>& attributes,
      ResourceState initial_state, ResourceUsage usage);
  std::shared_ptr<Buffer> getIndexBuffer(unsigned int index_count,
                                         IndexFormat index_format,
                                         ResourceState initial_state,
                                         ResourceUsage usage);
};

}  // namespace Resource
}  // namespace Renderer
}  // namespace CHCEngine