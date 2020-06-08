#pragma once
#include "CopyContext.h"
namespace CHCEngine {
namespace Renderer {
class Renderer;
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
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine