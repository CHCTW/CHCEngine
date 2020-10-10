#pragma once
#include <wrl/client.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../ClassName.h"
#include "Command.h"
#include "ContextResourceState.h"
using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
class Renderer;
class BaseFence;
namespace Resource {
class Resource;
class DynamicBuffer;
} // namespace Resource
namespace Pipeline {
class BindLayout;
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

  // why use raw point in the callback, cause shared ptr is not allowed in
  // the destructor, but when we wait we might still use in another thread,
  // that is when we wait in the destructor this will crashed
  // another way is use reference probally can chagne in future

  // going to add a {resource,end state},
  // when submit the context, will actually record the transition
  // commmand in another command list and submit
  enum class ContextState { CONTEXT_STATE_IDLE, CONTEXT_STATE_RECORDING };

protected:
  friend class ContextQueue;
  friend class Renderer;
  CommandType type_;
  std::shared_ptr<ContextCommand> context_command_;
  std::shared_ptr<ContextCommand> pending_transition_command_;
  std::shared_ptr<Pipeline::BindLayout> graphics_layout_;
  std::shared_ptr<Pipeline::BindLayout> compute_layout_;
  std::thread context_thread_;
  std::mutex wait_mutex_;
  std::atomic<ContextState> state_;
  template <class ContextClass>
  void running(std::function<void(ContextClass *)> funcs) {
    auto ptr = static_cast<ContextClass *>(this);
    funcs(ptr);
    state_.store(ContextState::CONTEXT_STATE_IDLE);
  }
  std::weak_ptr<ContextPoolBase> pool_;
  void resetContextCommand();
  void closeContext();
  void flushBarriers();
  std::vector<Transition> transitions_;
  std::vector<Transition> pending_transitons_;
  std::vector<UAVWait> uav_waits_;
  std::unordered_map<std::shared_ptr<Resource::Resource>, ContextResourceState>
      context_resource_states_;
  void updateContextResourceState(
      const std::shared_ptr<Resource::Resource> &resource,
      ResourceState next_state, bool split, uint32_t index);
  // run either pending or previous when submit context
  void resolvePendingResourceState();
  void resolvePreviousContextStateAndSelfClear(
      const std::shared_ptr<Context> &previours_context);

public:
  void waitRecordingDone();
  virtual ~Context();
  Context(CommandType type, std::shared_ptr<ContextCommand> context_command,
          std::weak_ptr<ContextPoolBase> pool)
      : type_(type), context_command_(std::move(context_command)), pool_(pool),
        state_(ContextState::CONTEXT_STATE_IDLE) {}
  template <class ContextClass>
  void recordCommands(std::function<void(ContextClass *)> funcs,
                      bool aysnc = true) {
    // waitRecordingDone();

    std::lock_guard<std::mutex> lock(wait_mutex_);
    if (context_thread_.joinable()) {
      // std::cout << "join wait" << std::endl;
      try {
        context_thread_.join();
      } catch (std::exception &ex) {
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
  CommandType getType();
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine