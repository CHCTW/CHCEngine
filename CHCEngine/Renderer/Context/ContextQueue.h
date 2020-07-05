#pragma once
#include <wrl/client.h>
#include <mutex>

#include "../ClassName.h"
#include "GraphicsContext.h"
#include "Fence.h"

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
  ContextQueue(ComPtr<CommandQueue> command_queue,CommandType type);
  ComPtr<CommandQueue> getCommandQueue();
  template<class ContextClass>
  void SubmitContextCommand(
      std::vector<std::shared_ptr<ContextClass>>& submit_context,
      const std::shared_ptr<ContextFence> & context_fence) {
    std::lock_guard<std::mutex> lock(submit_mutex_);
    std::vector<std::shared_ptr<ContextCommand>> execute_commands(
        submit_context.size());
    for (int i = 0; i < submit_context.size(); ++i) {
      //std::cout << "wait context recording cone" << std::endl;
      submit_context[i]->waitRecordingDone();
      if (!submit_context[i]->transitions_.empty()) {
        throw std::exception("Context has unrecorded resource transition");
      }
      //std::cout << "wait finish" << std::endl;
      submit_context[i]->closeContext();
      execute_commands[i] = submit_context[i]->context_command_;
      submit_context[i]->resetContextCommand();
    }
    context_fence->insertFenceSignal(command_queue_, execute_commands);
  }
  void insertFenceSignal(std::shared_ptr<ContextFence> context_fence) {
    std::lock_guard<std::mutex> lock(submit_mutex_);
    std::vector<std::shared_ptr<ContextCommand>> dummy;
    context_fence->insertFenceSignal(command_queue_, dummy);
  }
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine