#include "Context.h"
#include "../ClassName.h"

#include "../Renderer.h"
#include "Command.h"
#include "ContextPool.h"

namespace CHCEngine {
namespace Renderer {
namespace Context {
void Context::resetContextCommand() {
  if (!pool_.lock()) {
    throw std::exception("Invalid pool pointer while reseting !");
  }
  auto used_pool = pool_.lock();
  // should be reset after free at pool, when fence work complete, just simply
  // get here
  graphics_layout_.reset();
  compute_layout_.reset();
  context_command_ = used_pool->getContextCommand();
  context_resource_states_.clear();
  pending_transition_command_.reset();
  while (compute_bind_descriptors_.size())
    compute_bind_descriptors_.pop();
  while (graphics_bind_descriptors_.size())
    graphics_bind_descriptors_.pop();
}

void Context::closeContext() {
  flushBarriers();
  context_command_->close();
}

void Context::flushBarriers() {
  if (transitions_.size() || uav_waits_.size()) {
    context_command_->resrourceTransition(transitions_, uav_waits_);
    transitions_.clear();
    uav_waits_.clear();
  }
}
void Context::updateContextResourceState(
    const std::shared_ptr<Resource::Resource> &resource,
    ResourceState next_state, bool split, uint32_t index) {
  if (!context_resource_states_.count(resource)) {
    context_resource_states_.emplace(
        resource, ContextResourceState(resource->getSubResrouceCount()));
  }
  if (type_ == CommandType::COMMAND_TYPE_COPY)
    return;
  context_resource_states_[resource].stateUpdate(resource, next_state, split,
                                                 index, transitions_);
}
void Context::resolvePendingResourceState() {
  if (type_ == CommandType::COMMAND_TYPE_COPY)
    return;
  std::vector<Transition> pending_transitons;
  for (auto &rs : context_resource_states_) {
    rs.second.resovleResrouceState(rs.first, pending_transitons);
  }
  if (pending_transitons.size()) {
    auto use_pool = pool_.lock();
    if (use_pool) {
      pending_transition_command_ = use_pool->getContextCommand();
      std::vector<UAVWait> dummy;
      pending_transition_command_->resrourceTransition(pending_transitons,
                                                       dummy);
      pending_transition_command_->close();
    }
  }
  context_command_->referenceTrackingResrouce(context_resource_states_);
}
void Context::resolvePreviousContextStateAndSelfClear(
    const std::shared_ptr<Context> &previours_context) {
  if (type_ == CommandType::COMMAND_TYPE_COPY)
    return;
  std::vector<Transition> pending_transitons;
  for (auto &rs : context_resource_states_) {
    if (!previours_context->context_resource_states_.count(rs.first)) {
      previours_context->context_resource_states_[rs.first] =
          std::move(rs.second);
    } else {
      context_resource_states_[rs.first].addPreviousState(rs.first, rs.second,
                                                          pending_transitons);
    }
  }
  if (pending_transitons.size()) {
    auto use_pool = pool_.lock();
    if (use_pool) {
      pending_transition_command_ = use_pool->getContextCommand();
      std::vector<UAVWait> dummy;
      pending_transition_command_->resrourceTransition(pending_transitons,
                                                       dummy);
      pending_transition_command_->close();
    }
  }
}
void Context::addReferenceResrouceFromTracking() {
  context_command_->referenceTrackingResrouce(context_resource_states_);
}
void Context::waitRecordingDone() {
  try {
    std::lock_guard<std::mutex> lock(wait_mutex_);
    // std::cout << "test join" << std::endl;
    if (context_thread_.joinable()) {
      // std::cout << "join wait" << std::endl;
      context_thread_.join();
      // std::cout << "join wait finish" << std::endl;
    }
  } catch (std::exception &ex) {
    std::cerr << ex.what();
  }
  // std::cout << "test finish" << std::endl;
}

Context::~Context() {
  waitRecordingDone();
  if (pool_.lock()) {
    auto used_pool = pool_.lock();
    if (context_command_) {
      // will have free twice problem, not means it will caused
      // problem, but if release after submit to queue,
      // the context_command is actually unused but still going to
      // free
      // context_command_->reset();
      context_command_->close();
      context_command_->reset();
      used_pool->freeContextCommand(context_command_->id_);
      context_command_.reset();
    }
  }
}
CommandType Context::getType() { return type_; }
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine