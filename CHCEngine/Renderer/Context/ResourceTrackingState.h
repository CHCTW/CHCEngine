#pragma once
#include "../ClassName.h"
#include "../Resource/Resource.h"
#include "Command.h"

#include <memory>
#include <unordered_map>
namespace CHCEngine {
namespace Renderer {
namespace Context {

struct TrackingState {
  unsigned int sub_resrouces_count_ = 1;
  TrackingState(){};
  std::vector<SubResourceState> sub_resrouces_states_;
  // std::vector<bool>
  // std::unordered_map<ResourceState, unsigned int> state_counts_;
  bool allSameState() {
    if (sub_resrouces_count_)
      return true;
    SubResourceState first = sub_resrouces_states_[0];
    for (const auto &s : sub_resrouces_states_) {
      if (s != first)
        return false;
    }
    return true;
  }
  TrackingState(unsigned int total_sub_resrouces_count)
      : sub_resrouces_count_(total_sub_resrouces_count),
        sub_resrouces_states_(total_sub_resrouces_count) {
    // state_counts_[ResourceState::RESOURCE_STATE_UNKNOWN] =
    //   total_sub_resrouces_count;
  }
  // should have transition here also
  void
  updateAllResrouceState(SubResourceState all_resource_state,
                         const std::shared_ptr<Resource::Resource> &resource) {
    if (allSameState()) {
      auto current = sub_resrouces_states_[0];
      if (current == all_resource_state)
        return;
      //
    }
  }
  // same
  void updateSubResrouceState(unsigned int sub_resrouce_index,
                              SubResourceState sub_state) {
    /*if (sub_resrouce_index == all_subresrouce_index ||
        sub_resrouces_count_ == 1)
      sub_resrouces_states_.clear();
    sub_resrouces_states_[sub_resrouce_index] = sub_state;*/
  }
};

class ResrouceTrackingState {
private:
  std::unordered_map<std::shared_ptr<Resource::Resource>, TrackingState>
      resource_states_;

public:
  void resetAllTrackingState();
  // add apporiate transition to the transtions
  void addTransitionToList(std::vector<Transition> &transitions,
                           const std::shared_ptr<Resource::Resource> &resource,
                           unsigned int sub_resrouce_index,
                           ResourceState next_state,
                           ResourceTransitionFlag flag);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine