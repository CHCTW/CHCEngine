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
struct SubResourceTrackingState {
  SubResourceState sub_resrouces_state_;
  ResourceState first_transition_state_;
  bool mutable_ = true;
  bool merged_ = false;
  bool operator!=(const SubResourceTrackingState &r) {
    return sub_resrouces_state_ != r.sub_resrouces_state_ ||
           first_transition_state_ != r.first_transition_state_ ||
           mutable_ != mutable_ || merged_ != merged_;
  }
};
inline void addTransition(ResourceState before_state, ResourceState after_state,
                          unsigned int index, bool split,
                          const std::shared_ptr<Resource::Resource> &resource,
                          std::vector<Transition> &transitions) {
  ResourceTransitionFlag flag =
      ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_NONE;
  if (split)
    flag = ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_BEGIN;
  transitions.emplace_back(
      Transition{resource, before_state, after_state,
                 ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_END, index});
}

inline void updateState(ResourceState next_state, unsigned int index,
                        bool split, SubResourceTrackingState &current,
                        const std::shared_ptr<Resource::Resource> &resource,
                        std::vector<Transition> &transitions) {

  // spliting state case, means already set split before
  // that is said it won't be unknown
  if (split || !isReadState(next_state) ||
      current.sub_resrouces_state_.isTransiting())
    current.mutable_ = false;
  if (current.sub_resrouces_state_.isTransiting()) {
    transitions.emplace_back(Transition{
        resource, current.sub_resrouces_state_.previous_state_,
        current.sub_resrouces_state_.current_state_,
        ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_END, index});
    // not the same as we want, going to add another
    if (next_state != current.sub_resrouces_state_.current_state_) {
      addTransition(current.sub_resrouces_state_.current_state_, next_state,
                    index, split, resource, transitions);
    }
    // update tracking state
    if (split)
      current.sub_resrouces_state_.previous_state_ =
          current.sub_resrouces_state_.current_state_;
    else
      current.sub_resrouces_state_.previous_state_ = next_state;
    current.sub_resrouces_state_.current_state_ = next_state;
  } else {
    // normal case
    if (split && current.sub_resrouces_state_.current_state_ ==
                     ResourceState::RESOURCE_STATE_UNKNOWN) {
      std::string error =
          "Can't add split transition with untrack resoruce, at "
          "least need to use for once. Resrouce Name:";
      error += resource->getName();
      error += "subresrouce index : ";
      error += std::to_string(index);
      throw std::exception(error.c_str());
    }

    // already tracking, need to add transition unless it's the first time
    if (current.sub_resrouces_state_.current_state_ ==
        ResourceState::RESOURCE_STATE_UNKNOWN) {
      current.first_transition_state_ = next_state;
    } else if (current.sub_resrouces_state_.current_state_ != next_state) {
      if (current.mutable_) // can mute current state, just change state
      {

        next_state = mergeIfPossible(
            current.sub_resrouces_state_.current_state_, next_state);
        current.first_transition_state_ = next_state;
        current.merged_ = true;
      } else
        addTransition(current.sub_resrouces_state_.current_state_, next_state,
                      index, split, resource, transitions);
    }
    if (split)
      current.sub_resrouces_state_.previous_state_ =
          current.sub_resrouces_state_.current_state_;
    else
      current.sub_resrouces_state_.previous_state_ = next_state;
    current.sub_resrouces_state_.current_state_ = next_state;
  }
}
struct TrackingState {
  TrackingState(){};
  std::vector<SubResourceTrackingState> sub_tracking_states_;
  // std::unordered_map<ResourceState, unsigned int> state_counts_;
  inline bool allSameState() {
    if (sub_tracking_states_.size() == 1)
      return true;
    SubResourceTrackingState first = sub_tracking_states_[0];
    for (int i = 1; i < sub_tracking_states_.size(); ++i) {
      if (sub_tracking_states_[i] != first)
        return false;
    }
    return true;
  }
  TrackingState(unsigned int total_sub_resrouces_count)
      : sub_tracking_states_(total_sub_resrouces_count) {}
  // should have transition here also
  inline void
  updateAllResrouceState(const ResourceState &all_resource_state, bool split,
                         const std::shared_ptr<Resource::Resource> &resource,
                         std::vector<Transition> &transitions) {
    if (allSameState()) {
      // all the same, just use all subresource transition
      auto current = sub_tracking_states_[0];
      updateState(all_resource_state, all_subresrouce_index, split, current,
                  resource, transitions);
      std::fill(sub_tracking_states_.begin(), sub_tracking_states_.end(),
                current);
    } else {
      // different need to update one-by-one
      for (unsigned int i = 0; i < sub_tracking_states_.size(); ++i) {
        updateState(all_resource_state, i, split, sub_tracking_states_[i],
                    resource, transitions);
      }
    }
  }
  inline void
  updateSubResrouceState(const ResourceState &resource_state, bool split,
                         unsigned int sub_resrouce_index,
                         const std::shared_ptr<Resource::Resource> &resource,
                         std::vector<Transition> &transitions) {
    if (sub_resrouce_index == all_subresrouce_index) {
      updateAllResrouceState(resource_state, split, resource, transitions);
    } else {
      updateState(resource_state, sub_resrouce_index, split,
                  sub_tracking_states_[sub_resrouce_index], resource,
                  transitions);
    }
  }
  inline void addTransition(const std::shared_ptr<Resource::Resource> &resource,
                            std::vector<Transition> &transitions) {
    // add transition and update the state in the resource
  }
};

class ResrouceTrackingState {
private:
  std::unordered_map<std::shared_ptr<Resource::Resource>, TrackingState>
      resource_states_;

public:
  void resetAllTrackingState();
  // add apporiate transition to the transtions
  void updateResourceState(std::vector<Transition> &transitions,
                           const std::shared_ptr<Resource::Resource> &resource,
                           unsigned int sub_resrouce_index,
                           ResourceState next_state, bool split);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine