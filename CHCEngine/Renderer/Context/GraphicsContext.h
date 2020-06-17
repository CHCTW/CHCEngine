#pragma once
#include "ComputeContext.h"
namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Pipeline {
class Pipeline;
struct Viewport;
struct Scissor;
} // namespace Pipeline
namespace Resource {
class SwapChainBuffer;
class DynamicBuffer;
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
  void drawInstanced(unsigned int vertex_count, unsigned int instance_count = 1,
                     unsigned int start_vertex_location = 0,
                     unsigned int start_instance_location = 0);
  void setViewport(const Pipeline::Viewport &viewport);
  void setScissor(const Pipeline::Scissor &scissor);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine