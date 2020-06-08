#pragma once
#include "Context.h"
namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Context {
class CopyContext : public Context {
 protected:
  friend class Renderer;
 public:
  virtual ~CopyContext() {/* waitRecordingDone();*/ }
  CopyContext(CommandType type, std::shared_ptr<ContextCommand> context_command,
              std::weak_ptr<ContextPoolBase> pool)
      : Context(type, context_command, pool) {}
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine