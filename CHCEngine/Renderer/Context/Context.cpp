#include "../ClassName.h"
#include "Context.h"

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
  // should be reset after free at pool, just simply get here
  graphics_layout_.reset();
  compute_layout_.reset();
  context_command_ = used_pool->getContextCommand();
}

void Context::closeContext() { context_command_->close(); }

void Context::waitRecordingDone() {
  try {
    std::lock_guard<std::mutex> lock(wait_mutex_);
    // std::cout << "test join" << std::endl;
    if (context_thread_.joinable()) {
      // std::cout << "join wait" << std::endl;
      context_thread_.join();
      // std::cout << "join wait finish" << std::endl;
    }
  } catch (std::exception& ex) {
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
void Context::resourceTransition(std::shared_ptr<Resource::Resource> resource,
                                 ResourceState before_state,
                                 ResourceState after_state, bool set_barrier,
                                 ResourceTransitionFlag flag,
                                 unsigned int subresource_index) {
  transitions_.push_back(
      {resource, before_state, after_state, flag, subresource_index});
  if (set_barrier) {
    context_command_->resrourceTransition(transitions_);
    transitions_.clear();
  }
}
CommandType Context::getType() { return type_; }
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine