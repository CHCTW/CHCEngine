#include "../ClassName.h"
#include "ContextQueue.h"

namespace CHCEngine {
namespace Renderer {
namespace Context {

ContextQueue::ContextQueue(ComPtr<CommandQueue> command_queue,
                                         CommandType type)
    : command_queue_(std::move(command_queue)), type_(type) {}
ComPtr<CommandQueue> ContextQueue::getCommandQueue() {
  return command_queue_; }
//submit context with a fence, the fence and context will 
// wait until their previous work finish then start to submit
/*void ContextQueue::SubmitContextCommand<ContextClass>(
    std::vector<std::shared_ptr<Context>>& submit_context,
    std::shared_ptr<ContextFence> context_fence) {
  std::lock_guard<std::mutex> lock(submit_mutex_);
  std::vector<std::shared_ptr<ContextCommand>> execute_commands(
      submit_context.size());
  for (int i = 0; i < submit_context.size(); ++i) {
    if (!submit_context[i]->transitions_.empty()) {
      throw std::exception("Context has unrecorded resource transition");
    }
    submit_context[i]->waitRecordingDone();
    submit_context[i]->closeContext();
    execute_commands[i] = submit_context[i]->context_command_;
    submit_context[i]->resetContextCommand();
  }
  context_fence->insertFenceSignal(command_queue_, execute_commands);
}*/
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine