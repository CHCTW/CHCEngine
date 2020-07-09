#pragma once
#include <wrl/client.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "../ClassName.h"
#include "../Resource/Resource.h"
#include "Command.h"

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Context {
struct ContextCommand;
class FencePool;
class Renderer;
enum class FenceState { FENCE_STATE_IDLE, FENCE_STATE_WAITING };
class BaseFence {
  friend class ContextFence;
  friend class FencePool;
  friend class Renderer;

private:
  unsigned long long id_;
  FenceState state_;
  // when comes to wait in the queue, the value and the state
  // should be get at the same time, so we can choose the waiting
  // value.... should have another mutex for getting the value?
  ComPtr<Fence> fence_;
  HANDLE fence_event_;
  // a value will add after waiting, used for waiting
  UINT64 expected_value_;
  std::vector<std::shared_ptr<ContextCommand>> executing_commands_;
  std::thread waiting_thread_;
  // used for waiting value
  std::mutex wait_mutex_;
  std::mutex submit_mutex_;
  std::mutex state_value_mutex_;
  std::weak_ptr<FencePool> owner_;
  std::vector<std::shared_ptr<Resource::Resource>> using_resources_;
  FenceState getState();
  uint64_t getExpectedValue();
  std::pair<FenceState, uint64_t> getStateAndExpectedValue();

  void waitFenceComplete();
  void insertFenceWait(const ComPtr<CommandQueue> &command_queue,
                       uint64_t value);
  void insertFenceSignal(
      const ComPtr<CommandQueue> &command_queue,
      std::vector<std::shared_ptr<ContextCommand>> &execute_commands);
  void completeCallback();
  void free();
  void wait();

public:
  BaseFence(ComPtr<Fence> fence, unsigned long long id,
            std::weak_ptr<FencePool> owner);
  ~BaseFence();
};
// an interface to really exposed
class ContextFence {
private:
  std::shared_ptr<BaseFence> base_fence_;

public:
  ContextFence(std::shared_ptr<BaseFence> base_fence)
      : base_fence_(std::move(base_fence)) {}
  void waitComplete() { base_fence_->waitFenceComplete(); }
  ~ContextFence() { base_fence_->free(); }
  FenceState getState() { base_fence_->getState(); };
  void insertFenceWait(const ComPtr<CommandQueue> &command_queue,
                       uint64_t value) {
    base_fence_->insertFenceWait(command_queue, value);
  }

  void insertFenceSignal(
      const ComPtr<CommandQueue> &command_queue,
      std::vector<std::shared_ptr<ContextCommand>> &execute_commands) {
    base_fence_->insertFenceSignal(command_queue, execute_commands);
  }
  std::pair<FenceState, uint64_t> getStateAndExpectedValue() {
    return base_fence_->getStateAndExpectedValue();
  }
  
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine