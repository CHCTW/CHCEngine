#include "../ClassName.h"

#include "../Resource/SwapChainBuffer.h"
#include "GraphicsContext.h"
#include "../Pipeline/Pipeline.h"

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
void GraphicsContext::drawInstanced(unsigned int vertex_count,
                                    unsigned int instance_count,
                                    unsigned int start_vertex_location,
                                    unsigned int start_instance_location) {
  context_command_->drawInstanced(vertex_count, instance_count,
                                  start_vertex_location,
                                  start_instance_location);
}
void GraphicsContext::setViewport(const Pipeline::Viewport &viewport) {
  context_command_->setViewport(viewport);
}
void GraphicsContext::setScissor(const Pipeline::Scissor &scissor) {
  context_command_->setScissor(scissor);
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine