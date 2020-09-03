#pragma once
#include "../ClassName.h"
#include "../Resource/Resource.h"
#include "Command.h"

#include <memory>
#include <unordered_map>
namespace CHCEngine {
namespace Renderer {
namespace Context {

class ResrouceTrackingState {
private:
  std::unordered_map<std::shared_ptr<Resource::Resource>, TrackingState> resource_states_;

public:
  void resetAllTrackingState();
  // add apporiate transition to the transtions
  void addTransitionToList(std::vector<Transition> &transitions,
      const std::shared_ptr<Resource::Resource>& resource,
                           unsigned int sub_resrouce_index,
                           ResourceState next_state,
                           ResourceTransitionFlag flag);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine