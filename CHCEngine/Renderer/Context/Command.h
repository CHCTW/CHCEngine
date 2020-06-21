#pragma once
#include <memory>
#include <wrl/client.h>

#include "../ClassName.h"
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
class Buffer;
} // namespace Resource
namespace Context {
class ContextPoolBase;
class BaseFence;
struct Transition {
  std::shared_ptr<Resource::Resource> resource;
  ResourceState before_state;
  ResourceState after_state;
  ResourceTransitionFlag flag;
  unsigned int subresource_index;
};
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
struct ContextCommand {
  unsigned long long id_;
  friend class BaseFence;
  std::weak_ptr<ContextPoolBase> owner_;
  ComPtr<CommandAllocator> allocator_;
  ComPtr<CommandList> list_;
  std::vector<std::shared_ptr<Resource::Resource>> referenced_resources_;
  std::vector<std::shared_ptr<Resource::AllocateSpace>> allocated_spaces_;
  ComPtr<BindSignature> graphics_bind_signature_;
  ComPtr<PipelineState> pipeline_state_;
  ContextCommand(unsigned long long id, ComPtr<CommandAllocator> allocator,
                 ComPtr<CommandList> list, std::weak_ptr<ContextPoolBase> owner)
      : id_(id), allocator_(allocator), list_(list), owner_(owner) {}
  void free();
  void reset();
  void close();
  void resrourceTransition(std::vector<Transition> &transitions);
  void clearSwapChainBuffer(CPUDescriptorHandle handle, const float *color);
  void setPipelineState(ComPtr<PipelineState> pipeline_state);
  void updateBufferRegion(std::shared_ptr<Resource::Buffer>, void const *data,
                          unsigned long long data_byte_size,
                          unsigned long long offset);
  void updateBufferRegion(std::shared_ptr<Resource::Buffer>, void const *data,
                          unsigned long long data_byte_size,
                          unsigned long long offset,
                     std::shared_ptr<Resource::AllocateSpace> allocate_space);
  void drawInstanced(unsigned int vertex_count, unsigned int instance_count,
                     unsigned int start_vertex_location,
                     unsigned int start_instance_location);
  void setViewport(const Pipeline::Viewport &viewport);
  void setScissor(const Pipeline::Scissor &scissor);
  void setVertexBuffers(
      const std::vector<std::shared_ptr<Resource::Buffer>> &buffers);
  void setTopology(PrimitiveTopology topology);
  void setRenderTarget(CPUDescriptorHandle handle);
  void setGraphicsBindSignature(ComPtr<BindSignature> bind_signature);
  void bindGraphcisResource(std::shared_ptr<Resource::Resource> resource,
                            unsigned int usage_index,
                            unsigned int slot_index,
                            BindType bind_type, bool direct_bind);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine