#include "../ClassName.h"

#include "../Resource/Texture.h"
#include "ResourceTrackingState.h"
namespace CHCEngine {
namespace Renderer {
namespace Context {
void ResrouceTrackingState::resetAllTrackingState() {
  resource_states_.clear();
}

void ResrouceTrackingState::updateResourceState(
    std::vector<Transition> &transitions,
    const std::shared_ptr<Resource::Resource> &resource,
    unsigned int sub_resrouce_index, ResourceState next_state, bool split) {
  unsigned int total_sub_resource = resource->getSubResrouceCount();
  if (!resource_states_.count(resource)) {
    resource_states_.emplace(resource, TrackingState{total_sub_resource});
    return;
  }
  resource_states_[resource].updateSubResrouceState(
      next_state, split, sub_resrouce_index, resource, transitions);
}

} // namespace Context
} // namespace Renderer
} // namespace CHCEngine