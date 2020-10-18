#pragma once
#include <memory>
#include <wrl/client.h>

#include "../ClassName.h"
#include "ContextResourceState.h"
using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Pipeline {
class Pipeline;
struct Viewport;
struct Scissor;
} // namespace Pipeline
namespace Resource {
class Resource;
class Buffer;
struct AllocateSpace;
class Texture;
} // namespace Resource
namespace Sampler {
class Sampler;
}
namespace Context {
class ContextPoolBase;
class BaseFence;
class ContextResourceState;
struct UAVWait {
  std::shared_ptr<Resource::Resource> resource_;
};
using PendingTransitions =
    std::vector<Transition>[resrouce_transition_flag_count_];
struct ContextCommandAllocator {
  friend class CHCEngine::Renderer::Renderer;
  CommandAllocatorState state_;
  ComPtr<CommandAllocator> allocator_;
  ContextCommandAllocator(unsigned long long id,
                          ComPtr<CommandAllocator> allocator)
      : state_(CommandAllocatorState::COMMAND_ALLOCATOR_STATE_RECORD),
        allocator_(allocator) {}
  ContextCommandAllocator() = delete;
};
struct ContextCommandList {
  CommandListState state_;
  ComPtr<CommandList> list_;

  ContextCommandList(unsigned long long id, ComPtr<CommandList> list)
      : state_(CommandListState::COMMAND_LIST_STATE_RECORD), list_(list) {}
  ContextCommandList() = delete;
};
// thinking move all implementation to h file inline callback,
// since the call usually is a one line code

/*
The commands that will trigger the command to flush the barriers
this doesn't trigger pending barriers, only submit commands will trigger it

draw instance
draw instance index
depatch
execute indirect
occuluction query
clear rtv
clear dsv

This will record the barieers immedately
also close commands will trigger it immedately
*/

struct ContextCommand {
  unsigned long long id_;
  friend class BaseFence;
  std::weak_ptr<ContextPoolBase> owner_;
  ComPtr<CommandAllocator> allocator_;
  ComPtr<CommandList> list_;
  std::vector<std::shared_ptr<Resource::Resource>> referenced_resources_;
  std::vector<std::shared_ptr<Sampler::Sampler>> referenced_samplers_;
  std::vector<std::shared_ptr<Resource::AllocateSpace>> allocated_spaces_;
  ComPtr<BindSignature> graphics_bind_signature_;
  ComPtr<BindSignature> compute_bind_signature_;
  ComPtr<PipelineState> pipeline_state_;
  std::unordered_map<std::shared_ptr<Resource::Resource>, ContextResourceState>
      context_resource_states_;
  ContextCommand(unsigned long long id,
                 const ComPtr<CommandAllocator> &allocator,
                 const ComPtr<CommandList> &list,
                 std::weak_ptr<ContextPoolBase> owner)
      : id_(id), allocator_(allocator), list_(list), owner_(owner) {}
  void free();
  void reset();
  void close();
  void resrourceTransition(std::vector<Transition> &transitions,
                           std::vector<UAVWait> &waits);
  void clearRenderTarget(CPUDescriptorHandle handle, const float *color);
  void clearDepthStencil(CPUDescriptorHandle handle, float depth_value,
                         uint8_t stencil_value, bool depth, bool stencil);
  void setPipelineState(ComPtr<PipelineState> pipeline_state);
  void updateBufferRegion(std::shared_ptr<Resource::Buffer>, void const *data,
                          unsigned long long data_byte_size,
                          unsigned long long offset);
  void
  updateBufferRegion(std::shared_ptr<Resource::Buffer>, void const *data,
                     unsigned long long data_byte_size,
                     unsigned long long offset,
                     std::shared_ptr<Resource::AllocateSpace> &allocate_space);
  void drawInstanced(unsigned int vertex_count, unsigned int instance_count,
                     unsigned int start_vertex_location,
                     unsigned int start_instance_location);
  void setViewport(const Pipeline::Viewport &viewport);
  void setScissor(const Pipeline::Scissor &scissor);
  void setVertexBuffers(
      const std::vector<std::shared_ptr<Resource::Buffer>> &buffers);
  void setTopology(PrimitiveTopology topology);
  void setRenderTarget(CPUDescriptorHandle handle);
  void setRenderTarget(CPUDescriptorHandle *handles, uint32_t count,
                       CPUDescriptorHandle *depth_stencil_handle);
  void setGraphicsBindSignature(ComPtr<BindSignature> bind_signature);
  void setComputeBindSignature(ComPtr<BindSignature> bind_signature);
  void bindGraphicsResource(std::shared_ptr<Resource::Resource> resource,
                            unsigned int usage_index, unsigned int slot_index,
                            BindType bind_type, bool direct_bind);
  void bindGraphicsResource(const Resource::Resource *resource,
                            unsigned int usage_index, unsigned int slot_index,
                            BindType bind_type, bool direct_bind);
  void bindGraphicsConstants(const void *data, uint32_t slot_index,
                             uint32_t constant_count,
                             uint32_t constant_buffer_offset);

  void bindComputeResource(std::shared_ptr<Resource::Resource> resource,
                           unsigned int usage_index, unsigned int slot_index,
                           BindType bind_type, bool direct_bind);
  void bindComputeResource(const Resource::Resource *resource,
                           unsigned int usage_index, unsigned int slot_index,
                           BindType bind_type, bool direct_bind);
  void bindComputeConstants(const void *data, uint32_t slot_index,
                            uint32_t constant_count,
                            uint32_t constant_buffer_offset);

  void bindGraphicsSampler(std::shared_ptr<Sampler::Sampler> sampler,
                           unsigned int usage_index, unsigned int slot_index);

  void setStaticDescriptorHeap();
  void updateTextureRegion(
      std::shared_ptr<Resource::Texture> texture,
      const std::vector<D3D12_TEXTURE_COPY_LOCATION> &src_layouts,
      const std::vector<D3D12_TEXTURE_COPY_LOCATION> &dst_layouts);
  void updateTextureRegion(
      std::shared_ptr<Resource::Texture> texture,
      const std::vector<D3D12_TEXTURE_COPY_LOCATION> &src_layouts,
      const std::vector<D3D12_TEXTURE_COPY_LOCATION> &dst_layouts,
      std::vector<std::shared_ptr<Resource::AllocateSpace>> &spaces);
  void dispatch(unsigned int x, unsigned int y, unsigned int z);
  void referenceTrackingResrouce(
      std::unordered_map<std::shared_ptr<Resource::Resource>,
                         ContextResourceState> &context_resource_states);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine