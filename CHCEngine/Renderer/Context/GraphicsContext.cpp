#include "../ClassName.h"

#include "../Resource/SwapChainBuffer.h"
#include "GraphicsContext.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void GraphicsContext::clearRenderTarget(
    std::shared_ptr<Resource::SwapChainBuffer> swap_chain_buffer, Color color) {
  float c[4] = {color.r, color.g, color.b, color.a};
  /*c[0] = color.r;
  c[1] = color.g;
  c[2] = color.b;
  c[3] = color.a;*/
  context_command_->clearSwapChainBuffer(swap_chain_buffer->getDescriptor(), c);
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine