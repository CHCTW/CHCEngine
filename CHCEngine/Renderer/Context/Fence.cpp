#include "Fence.h"
#include "../ClassName.h"

#include "../D3D12Utilities.hpp"
#include "Command.h"
#include "FencePool.h"
#include <chrono>
#include <iostream>

namespace CHCEngine {
namespace Renderer {
namespace Context {
void BaseFence::insertFenceSignal(
    const ComPtr<CommandQueue> &command_queue,
    std::vector<std::shared_ptr<ContextCommand>> &execute_commands) {
  std::lock_guard<std::mutex> lock(submit_mutex_);
  // auto before = std::chrono::high_resolution_clock::now();
  waitFenceComplete();
  {
    std::lock_guard<std::mutex> lock(state_value_mutex_);
    state_ = FenceState::FENCE_STATE_WAITING;
  }
  std::vector<CommandListBase *> submit_list(execute_commands.size());
  for (int i = 0; i < execute_commands.size(); ++i) {
    submit_list[i] = execute_commands[i]->list_.Get();
    /*using_resources_.insert(using_resources_.begin(),
                            execute_commands[i]->referenced_resources_.begin(),
                            execute_commands[i]->referenced_resources_.end());*/
  }
  executing_commands_ = std::move(execute_commands); // copy to fence,

  if (!submit_list.empty()) {
    command_queue->ExecuteCommandLists(static_cast<UINT>(submit_list.size()),
                                       submit_list.data());
  }
  UINT64 wait_value = 0;
  {
    std::lock_guard<std::mutex> lock(state_value_mutex_);
    wait_value = expected_value_;
  }
  ThrowIfFailed(command_queue->Signal(fence_.Get(), wait_value));
  waiting_thread_ = std::thread(&BaseFence::wait, this);
  // auto delta = std::chrono::high_resolution_clock::now() - before;
  // std::cout << "fence waiting time" << delta.count() / 1000000 << std::endl;
}
void BaseFence::completeCallback() {
  for (auto &command : executing_commands_) {
    command->free();
  }
  using_resources_.clear();
  executing_commands_.clear();
  {
    std::lock_guard<std::mutex> lock(state_value_mutex_);
    state_ = FenceState::FENCE_STATE_IDLE;
    ++expected_value_;
  }
}
void BaseFence::free() {
  if (auto use_owner = owner_.lock()) {
    use_owner->freeBaseFence(id_);
  }
}
void BaseFence::wait() {
  ThrowIfFailed(fence_->SetEventOnCompletion(expected_value_, fence_event_));
  WaitForSingleObject(fence_event_, INFINITE);
  completeCallback();
}
FenceState BaseFence::getState() {
  std::lock_guard<std::mutex> lock(state_value_mutex_);
  return state_;
}
uint64_t BaseFence::getExpectedValue() {
  std::lock_guard<std::mutex> lock(state_value_mutex_);
  return expected_value_;
}
std::pair<FenceState, uint64_t> BaseFence::getStateAndExpectedValue() {
  std::lock_guard<std::mutex> lock(state_value_mutex_);
  return {state_, expected_value_};
}
void BaseFence::waitFenceComplete() {
  std::lock_guard<std::mutex> lock(wait_mutex_);
  if (waiting_thread_.joinable()) {
    waiting_thread_.join();
  }
}
void BaseFence::insertFenceWait(const ComPtr<CommandQueue> &command_queue,
                                uint64_t value) {
  command_queue->Wait(fence_.Get(), value);
}
BaseFence::BaseFence(ComPtr<Fence> fence, unsigned long long id,
                     std::weak_ptr<FencePool> owner)
    : fence_(std::move(fence)), expected_value_(1),
      state_(FenceState::FENCE_STATE_IDLE), id_(id), owner_(owner) {
  fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (fence_event_ == nullptr) {
    ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
  }
}
BaseFence::~BaseFence() { waitFenceComplete(); }
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine