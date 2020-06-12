#pragma once
#include "CopyContext.h"

namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Pipeline {
class Pipeline;
}
namespace Context {
class ComputeContext : public CopyContext {
 protected:
  friend class Renderer;
 public:
  virtual ~ComputeContext() { 
     // waitRecordingDone(); 
  }
  ComputeContext(CommandType type,
                 std::shared_ptr<ContextCommand> context_command,
              std::weak_ptr<ContextPoolBase> pool)
      : CopyContext(type, context_command, pool) {}
  void setPipeline(std::shared_ptr<Pipeline::Pipeline> pipline);
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine