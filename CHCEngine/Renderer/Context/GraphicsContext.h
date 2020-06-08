#pragma once
#include "ComputeContext.h"
namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Resource {
class SwapChainBuffer;
}
namespace Context {
class GraphicsContext : public ComputeContext {
protected:
  friend class Renderer;

public:
  ~GraphicsContext() {
    // waitRecordingDone();
  }
  GraphicsContext(CommandType type,
                  std::shared_ptr<ContextCommand> context_command,
                  std::weak_ptr<ContextPoolBase> pool)
      : ComputeContext(type, context_command, pool) {}
  void clearRenderTarget(
      std::shared_ptr<Resource::SwapChainBuffer> swap_chain_buffer,
      Color color);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine