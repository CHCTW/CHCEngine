#include "../ClassName.h"
#include "Command.h"

#include "../D3D12Utilities.hpp"
#include "ContextPool.h"
#include "../Resource/Resource.h"
#include "../D3D12Convert.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void ContextCommand::free() {
  referenced_resources_.clear();
  if (auto use_owner = owner_.lock()) {
    reset();
    use_owner->freeContextCommand(id_);
  }
}

void ContextCommand::reset() {
  ThrowIfFailed(allocator_->Reset());
  ThrowIfFailed(list_->Reset(allocator_.Get(), nullptr));
}

void ContextCommand::close() { ThrowIfFailed(list_->Close()); }

void ContextCommand::resrourceTransition(std::vector<Transition>& transitions) {
  std::vector<D3D12_RESOURCE_BARRIER> barriers_(transitions.size());
  for (int i = 0; i < transitions.size(); ++i) {
    referenced_resources_.emplace_back(transitions[i].resource);
    barriers_[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers_[i].Flags =
        convertToD3D12ResourceBarrierFlags(transitions[i].flag);
    barriers_[i].Transition.pResource =
        transitions[i].resource->getGPUResource().Get();
    barriers_[i].Transition.StateBefore =
        convertToD3D12ResourceStates(transitions[i].before_state);
    barriers_[i].Transition.StateAfter =
        convertToD3D12ResourceStates(transitions[i].after_state);
    barriers_[i].Transition.Subresource = transitions[i].subresource_index;
  }
  list_->ResourceBarrier(static_cast<unsigned int>(transitions.size()), barriers_.data());
}
void ContextCommand::clearSwapChainBuffer(CPUDescriptorHandle handle,
                                          const float* color) {
  list_->ClearRenderTargetView(handle, color,0,nullptr);
}
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine