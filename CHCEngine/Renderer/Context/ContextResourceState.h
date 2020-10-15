#pragma once
#include "../ClassName.h"

#include "../Resource/Resource.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
namespace CHCEngine {
namespace Renderer {
namespace Context {
class ContextSubResourceState : public SubResourceState {
private:
  ResourceState first_transition_state_ = ResourceState::RESOURCE_STATE_UNKNOWN;
  bool mutable_ = true;
  bool merged_ = false;
  void addTransition(ResourceState before_state, ResourceState after_state,
                     uint32_t index, bool split,
                     const std::shared_ptr<Resource::Resource> &resource,
                     std::vector<Transition> &transitions);
  void
  resolveSplitTransition(uint32_t index,
                         const std::shared_ptr<Resource::Resource> &resource,
                         std::vector<Transition> &transitions);
  void updateMutable(ResourceState next_state, bool split);
  bool isTracked();
  bool needUpdate(ResourceState next_state);
  void nextStateValidCheck(const std::shared_ptr<Resource::Resource> &resource,
                           ResourceState next_state, bool split,
                           uint32_t index);
  void stateUpdateMutable(ResourceState next_state);
  void stateUpdateInmutable(const std::shared_ptr<Resource::Resource> &resource,
                            ResourceState next_state, bool split,
                            uint32_t index,
                            std::vector<Transition> &transitions);
  bool needResolveDecayResrouceState(
      const std::shared_ptr<Resource::Resource> &resource,
      SubResourceState &sub_resource_state);
  void reset();

public:
  bool operator!=(const ContextSubResourceState &r) const {
    return previous_state_ != r.previous_state_ ||
           current_state_ != r.current_state_ ||
           first_transition_state_ != r.first_transition_state_ ||
           mutable_ != mutable_ || merged_ != merged_;
  }
  ContextSubResourceState &
  operator=(const ContextSubResourceState &r) = default;
  void stateUpdate(ResourceState next_state, uint32_t index, bool split,
                   const std::shared_ptr<Resource::Resource> &resource,
                   std::vector<Transition> &transitions);
  void
  resovleSubResrouceState(const std::shared_ptr<Resource::Resource> &resource,
                          SubResourceState &sub_resource_state, uint32_t index,
                          std::vector<Transition> &transitions);
  void addPreviousState(const std::shared_ptr<Resource::Resource> &resource,
                        ContextSubResourceState &previous_context_state,
                        uint32_t index, std::vector<Transition> &transitions);
};
class ContextResourceState {
private:
  std::vector<ContextSubResourceState> context_sub_resrouce_states_;
  bool same_states_ = true;
  void checkSameStates(uint32_t start_index = 0);
  void
  stateUpateAllSubResource(const std::shared_ptr<Resource::Resource> &resource,
                           ResourceState next_state, bool split,
                           std::vector<Transition> &transitions);
  void
  resovleAllResrouceState(const std::shared_ptr<Resource::Resource> &resource,
                          std::vector<Transition> &transitions);

public:
  ContextResourceState() = default;
  ContextResourceState(uint32_t size) : context_sub_resrouce_states_(size){};
  void stateUpdate(const std::shared_ptr<Resource::Resource> &resource,
                   ResourceState next_state, bool split, uint32_t index,
                   std::vector<Transition> &transitions);
  void resovleResrouceState(const std::shared_ptr<Resource::Resource> &resource,
                            std::vector<Transition> &transitions);
  void addPreviousState(const std::shared_ptr<Resource::Resource> &resource,
                        ContextResourceState &previous_state,
                        std::vector<Transition> &transitions);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine