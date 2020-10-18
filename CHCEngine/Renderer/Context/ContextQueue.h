#pragma once
#include "../ClassName.h"

#include <mutex>
#include <wrl/client.h>

#include "Fence.h"
#include "GraphicsContext.h"

using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
namespace Context {
class ContextQueue {
private:
  ComPtr<CommandQueue> command_queue_;
  CommandType type_;
  std::mutex submit_mutex_;

public:
  ContextQueue(ComPtr<CommandQueue> command_queue, CommandType type);
  ComPtr<CommandQueue> getCommandQueue();
  template <class ContextClass>
  void SubmitContextCommand(
      const std::vector<std::shared_ptr<ContextClass>> &submit_context,
      const std::shared_ptr<ContextFence> &context_fence) {
    std::lock_guard<std::mutex> lock(submit_mutex_);
    for (int i = 0; i < submit_context.size(); ++i) {
      submit_context[i]->waitRecordingDone();
      submit_context[i]->closeContext();
    }
    uint32_t total_commands_count = 0;
    for (uint32_t i = 1; i < submit_context.size(); ++i) {
      submit_context[i]->resolvePreviousContextStateAndSelfClear(
          submit_context[0]);
      if (submit_context[i]->pending_transition_command_)
        total_commands_count += 2;
      else
        ++total_commands_count;
    }
    submit_context[0]->resolvePendingResourceState();
    submit_context[0]->addReferenceResrouceFromTracking();
    if (submit_context[0]->pending_transition_command_)
      total_commands_count += 2;
    else
      ++total_commands_count;
    std::vector<std::shared_ptr<ContextCommand>> execute_commands(
        total_commands_count);
    uint32_t count = 0;
    for (int i = 0; i < submit_context.size(); ++i) {
      if (submit_context[i]->pending_transition_command_) {

        execute_commands[count] =
            std::move(submit_context[i]->pending_transition_command_);
        ++count;
      }
      execute_commands[count] = std::move(submit_context[i]->context_command_);
      ++count;
      submit_context[i]->resetContextCommand();
    }
    context_fence->insertFenceSignal(command_queue_, execute_commands);
  }
  template <class ContextClass>
  void waitFenceSubmitContextCommand(
      const std::shared_ptr<ContextFence> &wait_fence, uint64_t value,
      const std::vector<std::shared_ptr<ContextClass>> &submit_context,
      const std::shared_ptr<ContextFence> &context_fence) {
    std::lock_guard<std::mutex> lock(submit_mutex_);
    for (int i = 0; i < submit_context.size(); ++i) {
      submit_context[i]->waitRecordingDone();
      submit_context[i]->closeContext();
    }
    uint32_t total_commands_count = 0;
    for (uint32_t i = 1; i < submit_context.size(); ++i) {
      submit_context[i]->resolvePreviousContextStateAndSelfClear(
          submit_context[0]);
      if (submit_context[i]->pending_transition_command_)
        total_commands_count += 2;
      else
        ++total_commands_count;
    }
    submit_context[0]->resolvePendingResourceState();
    submit_context[0]->addReferenceResrouceFromTracking();
    if (submit_context[0]->pending_transition_command_)
      total_commands_count += 2;
    else
      ++total_commands_count;
    std::vector<std::shared_ptr<ContextCommand>> execute_commands(
        total_commands_count);
    uint32_t count = 0;
    for (int i = 0; i < submit_context.size(); ++i) {
      if (submit_context[i]->pending_transition_command_) {

        execute_commands[count] =
            std::move(submit_context[i]->pending_transition_command_);
        ++count;
      }
      execute_commands[count] = std::move(submit_context[i]->context_command_);
      ++count;
      submit_context[i]->resetContextCommand();
    }
    wait_fence->insertFenceWait(command_queue_, value);
    context_fence->insertFenceSignal(command_queue_, execute_commands);
  }

  void insertFenceSignal(const std::shared_ptr<ContextFence> &context_fence) {
    std::lock_guard<std::mutex> lock(submit_mutex_);
    std::vector<std::shared_ptr<ContextCommand>> dummy;
    context_fence->insertFenceSignal(command_queue_, dummy);
  }
  void insertWaitFenceValue(const std::shared_ptr<ContextFence> &context_fence,
                            uint64_t value) {
    context_fence->insertFenceWait(command_queue_, value);
  }
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine