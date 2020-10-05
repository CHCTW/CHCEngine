#include "../ClassName.h"

#include "ContextResrouceState.h"

namespace CHCEngine {
namespace Renderer {
namespace Context {
void ContextSubResrouceState::addTransition(
    ResourceState before_state, ResourceState after_state, uint32_t index,
    bool split, const std::shared_ptr<Resource::Resource> &resource,
    std::vector<Transition> &transitions) {
  ResourceTransitionFlag flag =
      ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_NONE;
  if (split)
    flag = ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_BEGIN;
  transitions.emplace_back(
      Transition{resource, before_state, after_state, flag, index});
}

void ContextSubResrouceState::resolveSplitTransition(
    uint32_t index, bool split,
    const std::shared_ptr<Resource::Resource> &resource,
    std::vector<Transition> &transitions) {
  if (!isTracked() || !isTransiting())
    return;
  transitions.emplace_back(
      Transition{resource, previous_state_, current_state_,
                 ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_END, index});
  previous_state_ = current_state_;

} // namespace Context
void ContextSubResrouceState::updateMutable(ResourceState next_state,
                                            bool split) {
  if (split || !isReadState(next_state) || isTransiting())
    mutable_ = false;
}
bool ContextSubResrouceState::isTracked() {
  if (current_state_ == ResourceState::RESOURCE_STATE_UNKNOWN)
    return false;
  return true;
}
bool ContextSubResrouceState::needUpdate(ResourceState next_state) {
  // when we need to update the
  return needChange(next_state, current_state_);
}
void ContextSubResrouceState::nextStateValidCheck(
    const std::shared_ptr<Resource::Resource> &resource,
    ResourceState next_state, bool split, uint32_t index) {
  if (split && !isTracked()) {
    std::string error =
        "Can't add split transition with untraceable resource, at "
        "least need to use for once. Resource Name:";
    error += resource->getName();
    error += "sub resource index : ";
    error += std::to_string(index);
    throw std::exception(error.c_str());
  }
}
void ContextSubResrouceState::stateUpdateMutable(ResourceState next_state) {
  // when running to this line
  // some conditioning must be assumed
  // 1. next state and current state is one of the
  //    read bit combination, or current state is unknown
  // 2. must be in none split transition

  // none tracked case, just update the state
  ResourceState state = next_state;
  if (isTracked()) {
    // calculate merged bit
    state = mergeIfPossible(current_state_, next_state);
    merged_ = true;
  }
  current_state_ = state;
  previous_state_ = state;
  first_transition_state_ = state;
}
void ContextSubResrouceState::stateUpdateInmutable(
    const std::shared_ptr<Resource::Resource> &resource,
    ResourceState next_state, bool split, uint32_t index,
    std::vector<Transition> &transitions) {

  if (!isTracked()) {
    // untracked before, need to update first state
    first_transition_state_ = next_state;
  }
  addTransition(current_state_, next_state, index, split, resource,
                transitions);
  if (split)
    previous_state_ = current_state_;
  else
    previous_state_ = next_state;
  current_state_ = next_state;
}
void ContextSubResrouceState::stateUpdate(
    ResourceState next_state, uint32_t index, bool split,
    const std::shared_ptr<Resource::Resource> &resource,
    std::vector<Transition> &transitions) {
  nextStateValidCheck(resource, next_state, split, index);
  resolveSplitTransition(index, split, resource, transitions);
  updateMutable(next_state, split);
  if (!needUpdate(next_state))
    return;
  if (mutable_) {    stateUpdateMutable(next_state);
  } else {
    stateUpdateInmutable(resource, next_state, split, index, transitions);
  }
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine