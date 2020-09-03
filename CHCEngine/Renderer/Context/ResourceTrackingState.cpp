#include "../ClassName.h"

#include "ResourceTrackingState.h"
namespace CHCEngine {
namespace Renderer {
namespace Context {
void ResrouceTrackingState::resetAllTrackingState() {
  resource_states_.clear();
}

void ResrouceTrackingState::addTransitionToList(
    std::vector<Transition> &transitions,
    const std::shared_ptr<Resource::Resource> &resource,
    unsigned int sub_resrouce_index, ResourceState next_state,
    ResourceTransitionFlag flag) {
  if (!resource_states_.count(resource)) {
    // didn't find the resource, not traking at all, just going to add 
      // into the trakcing
    SubResourceState track;
    track.state_ = next_state;
    if (flag == ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_BEGIN)
      track.transition_state_ = TranstionState::TRANSITION_STATE_TRANSITING;
    resource_states_[resource].setSubResrouceState(sub_resrouce_index, track);
    return;
  } else {
      // has tracking, transition all cases
      if (sub_resrouce_index == all_subresrouce_index) {
        //if (resource_states_[resource])
          
      }

  }
}

} // namespace Context
} // namespace Renderer
} // namespace CHCEngine