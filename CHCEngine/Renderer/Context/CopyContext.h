#pragma once
#include "Context.h"

#include <memory>

namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Resource {
class Buffer;
}
namespace Context {
class CopyContext : public Context {
protected:
  friend class Renderer;

public:
  virtual ~CopyContext() { /* waitRecordingDone();*/
  }
  CopyContext(CommandType type, std::shared_ptr<ContextCommand> context_command,
              std::weak_ptr<ContextPoolBase> pool)
      : Context(type, context_command, pool) {}
  void updateBuffer(std::shared_ptr<Resource::Buffer> buffer, void *const data,
                    unsigned long long data_byte_size,
                    unsigned long long offset = 0);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine