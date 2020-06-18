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
class Buffer;
} // namespace Resource
namespace Pipeline
{
class BindLayout;
}
namespace Context {
class GraphicsContext : public ComputeContext {
protected:
  friend class Renderer;
  std::shared_ptr<Pipeline::BindLayout> graphics_layout_;
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
  void
  setVertexBuffers(const std::vector<std::shared_ptr<Resource::Buffer>> & buffers);
  template <typename... T> void setVertexBuffers(T &&... buffers) {
    setVertexBuffers({std::forward<T>(buffers)...});
  }
  void setPrimitiveTopology(PrimitiveTopology topology);
  void
  setRenderTarget(std::shared_ptr<Resource::SwapChainBuffer> swap_chain_buffer);
  // unfinished, should also  bind the resrouce in the bind layout ,will add it later
  void setGraphicsBindLayout(std::shared_ptr<Pipeline::BindLayout> bind_layout);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine