#pragma once
#include "../ClassName.h"

#include "../Resource/Resource.h"
#include "Command.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
namespace CHCEngine {
namespace Renderer {
namespace Context {
struct ContextSubResrouceState : public SubResourceState {
  ResourceState first_transition_state_ = ResourceState::RESOURCE_STATE_UNKNOWN;
  bool mutable_ = true;
  bool merged_ = false;
  bool operator!=(const ContextSubResrouceState &r) {
    return previous_state_ == r.previous_state_ &&
               current_state_ == r.current_state_ ||
           first_transition_state_ != r.first_transition_state_ ||
           mutable_ != mutable_ || merged_ != merged_;
  }
  void addTransition(ResourceState before_state, ResourceState after_state,
                     uint32_t index, bool split,
                     const std::shared_ptr<Resource::Resource> &resource,
                     std::vector<Transition> &transitions);
  void
  resolveSplitTransition(uint32_t index, bool split,
                         const std::shared_ptr<Resource::Resource> &resource,
                         std::vector<Transition> &transitions);
  void updateMutable(ResourceState next_state, bool split);
  bool isTracked();
  bool needUpdate(ResourceState next_state);
  void nextStateValidCheck(const std::shared_ptr<Resource::Resource> &resource,
                           ResourceState next_state, bool split,
                           uint32_t index);
  void stateUpdate(ResourceState next_state, uint32_t index, bool split,
                   const std::shared_ptr<Resource::Resource> &resource,
                   std::vector<Transition> &transitions);
  void stateUpdateMutable(ResourceState next_state);
  void stateUpdateInmutable(const std::shared_ptr<Resource::Resource> &resource,
                            ResourceState next_state, bool split,
                            uint32_t index,
                            std::vector<Transition> &transitions);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine