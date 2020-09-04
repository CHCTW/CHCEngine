#include "../ClassName.h"

#include "../Resource/Texture.h"
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
  unsigned int total_sub_resource = 1;
  // only texture will have subresources
  if (resource->getType() == Resource::ResourceType::RESOURCE_TYPE_TEXTURE) {
    Resource::Texture *texture =
        static_cast<Resource::Texture *>(resource.get());
    total_sub_resource =
        (texture->getTextureInformation()).getSubResrouceCount();
  }
  if (!resource_states_.count(resource)) {
    // didn't find the resource, not traking at all, just going to add
    // into the trakcing
    SubResourceState track;
    track.current_state_ = next_state;
    // if it's a split, it will use have different state
    if (flag != ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_BEGIN)
      track.previous_state_ = next_state;
    resource_states_[resource].setSubResrouceCount(total_sub_resource);
    resource_states_[resource].updateSubResrouceState(sub_resrouce_index, track);
    return;
  } else {
    // has tracking, transition all cases
    if (sub_resrouce_index == all_subresrouce_index) {
      if (resource_states_[resource].allSubResrouceSameState()) {
        // all resource in one state, add one transition
        // just like normal update subresource index

      } else {
          // loop all the index sub index, genearte 1,2 transition 
      }
    }
  }
}

} // namespace Context
} // namespace Renderer
} // namespace CHCEngine