#pragma once
#include "CopyContext.h"

namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Resource {
class DynamicBuffer;
} // namespace Resource
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
  void setPipeline(const std::shared_ptr<Pipeline::Pipeline> &pipline);
  void setStaticUsageHeap();
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine