#pragma once

#include <wrl/client.h>

#include "../DescriptorHeap.h"
#include "Buffer.h"
#include "Texture.h"
#include "ResourceGroup.h"
#include <mutex>
#include <unordered_map>

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
  std::shared_ptr<DescriptorHeap> shader_visible_resource_heap_;
  std::shared_ptr<DescriptorHeap> shader_visible_sampler_heap_;
  /*ComPtr<GPUResource> createBuffer(UINT64 size, HeapType heap_type,
                                   ResourceState initial_state);*/
  std::unordered_map<BufferType, unsigned long long> buffer_id_count_;
  std::unordered_map<TextureType, unsigned long long> texture_id_count_;
  unsigned long long resource_group_id_count_;
  std::mutex pool_mutex_;
  unsigned long long getNextBufferId(BufferType type);
  unsigned long long getNextTextureId(TextureType type);
  unsigned long long getNextResourceGroupId();

public:
  ResourcePool(ComPtr<Device> device,
               std::shared_ptr<DescriptorHeap> srv_uav_cbv,
               std::shared_ptr<DescriptorHeap> rtv,
               std::shared_ptr<DescriptorHeap> dsv,
               std::shared_ptr<DescriptorHeap> shader_visible_resource_heap,
               std::shared_ptr<DescriptorHeap> shader_visible_sampler_heap);
  // we assume the vertex buffer is wait for upload
  std::shared_ptr<Buffer> getVertexBuffer(
      unsigned int vertex_count,
      const std::vector<std::pair<std::string, DataFormat>> &attributes,
      ResourceState initial_state, ResourceUpdateType update_type);
  std::shared_ptr<Buffer> getIndexBuffer(unsigned int index_count,
                                         IndexFormat index_format,
                                         ResourceState initial_state,
                                         ResourceUpdateType update_type);
  std::shared_ptr<Buffer>
  getBuffer(unsigned int element_count, unsigned int element_byte_size,
            const std::vector<BufferUsage> &usages,
            const std::vector<std::pair<std::string, DataFormat>> &attributes,
            IndexFormat index_format, ResourceState initial_state,
            ResourceUpdateType update_type);
  std::shared_ptr<Texture>
  getTexture(TextureType texture_type, RawFormat raw_format,
             unsigned long long width, unsigned int height, unsigned int depth,
             unsigned int mip_levels, const std::vector<TextureUsage> &usages,
             const std::vector<RenderTargetUsage> &render_target_usages,
             const std::vector<DepthStencilUsage> &depth_stencil_usages,
      ResourceState initial_state, ResourceUpdateType update_type);
  std::shared_ptr<ResourceGroup> getResrouceGroup(unsigned int size);
};

} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine