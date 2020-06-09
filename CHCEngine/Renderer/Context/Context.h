#pragma once
#include <wrl/client.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "../ClassName.h"
#include "Command.h"
using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
class Renderer;
class BaseFence;
namespace Resource {
class Resource;
}
namespace Context {
class ContextPoolBase;
// the base class, context will have  three level inheritance
// copy->compute->graphics : copy context only will have copy command
// bundle shold be the same as graphics so might inheritance from graphics
class Context : public std::enable_shared_from_this<Context> {
  // idle means can directly use this context to record command
  // recording means the thead is running , when ever the thead_id is not
  // context_thead_, when set for any command will check first and wait for
  // thead done when submit to queue, this will caused wait the state until it's
  // idle
  // for command, when it pocessed, it can record, when it submiet it will
  // set command to nullptr, fence will handel return the comamnd to
  // pool
  enum class ContextState { CONTEXT_STATE_IDLE, CONTEXT_STATE_RECORDING };

 protected:
  friend class ContextQueue;
  friend class Renderer;
  CommandType type_;
  std::shared_ptr<ContextCommand> context_command_;
  std::thread context_thread_;
  std::mutex wait_mutex_;
  std::atomic<ContextState> state_;
  template <class ContextClass>
  void running(std::function<void(ContextClass*)> funcs) {
    auto ptr = static_cast<ContextClass*>(this);
    funcs(ptr);
    state_.store(ContextState::CONTEXT_STATE_IDLE);
  }
  std::weak_ptr<ContextPoolBase> pool_;
  void resetContextCommand();
  void closeContext();
  std::vector<Transition> transitions_;
 public:
  void waitRecordingDone();
  virtual ~Context();
  Context(CommandType type, std::shared_ptr<ContextCommand> context_command,
          std::weak_ptr<ContextPoolBase> pool)
      : type_(type),
        context_command_(context_command),
        pool_(pool),
        state_(ContextState::CONTEXT_STATE_IDLE) {}
  template <class ContextClass>
  void recordCommands(std::function<void(ContextClass*)> funcs,
                      bool aysnc = true) {
    // waitRecordingDone();

    std::lock_guard<std::mutex> lock(wait_mutex_);
    if (context_thread_.joinable()) {
      // std::cout << "join wait" << std::endl;
      try {
        context_thread_.join();
      } catch (std::exception& ex) {
        std::cerr << ex.what();
      }
      // std::cout << "join wait finish" << std::endl;
    }
    state_.store(ContextState::CONTEXT_STATE_RECORDING);
    if (aysnc)
      context_thread_ =
          std::thread(&Context::running<ContextClass>, this, funcs);
    else
      this->running<ContextClass>(funcs);
  }
  void resourceTransition(
      std::shared_ptr<Resource::Resource> resource, ResourceState before_state,
      ResourceState after_state, bool set_barrier = false,
      ResourceTransitionFlag flag =
          ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_NONE,
      unsigned int subresource_index = all_subresrouce_index);
  CommandType getType();
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine