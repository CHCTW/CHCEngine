#pragma once
#include <wrl/client.h>
#include <memory>

#include "../ClassName.h"
using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Resource {
class Resource;
}
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
      : state_(CommandListState::COMMAND_LIST_STATE_RECORD),
        list_(list) {}
  ContextCommandList() = delete;
};
struct ContextCommand {
  unsigned long long id_;
  friend class BaseFence;
  std::weak_ptr<ContextPoolBase> owner_;
  ComPtr<CommandAllocator> allocator_;
  ComPtr<CommandList> list_;
  std::vector<std::shared_ptr<Resource::Resource>> referenced_resources_;
  ContextCommand(unsigned long long id, ComPtr<CommandAllocator> allocator,
                 ComPtr<CommandList> list, std::weak_ptr<ContextPoolBase> owner)
      : id_(id), allocator_(allocator), list_(list),owner_(owner) {}
  void free();
  void reset();
  void close();
  void resrourceTransition(std::vector<Transition>& transitions);
  void clearSwapChainBuffer(CPUDescriptorHandle handle, const float* color);
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine