#include "../ClassName.h"

#include "../Pipeline/Pipeline.h"
#include "../Resource/SwapChainBuffer.h"
#include "GraphicsContext.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void GraphicsContext::clearRenderTarget(
    const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer,
    Color color) {
  float c[4] = {color.r, color.g, color.b, color.a};
  /*c[0] = color.r;
  c[1] = color.g;
  c[2] = color.b;
  c[3] = color.a;*/
  updateContextResourceState(
      swap_chain_buffer, ResourceState::RESOURCE_STATE_RENDER_TARGET, false, 0);
  flushBarriers();
  context_command_->clearSwapChainBuffer(swap_chain_buffer->getDescriptor(), c);
}
void GraphicsContext::drawInstanced(unsigned int vertex_count,
                                    unsigned int instance_count,
                                    unsigned int start_vertex_location,
                                    unsigned int start_instance_location) {
  flushBarriers();
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
void GraphicsContext::setVertexBuffers(
    const std::vector<std::shared_ptr<Resource::Buffer>> &buffers) {
  context_command_->setVertexBuffers(buffers);
}
void GraphicsContext::setPrimitiveTopology(PrimitiveTopology topology) {
  context_command_->setTopology(topology);
}
void GraphicsContext::setRenderTarget(
    const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer) {
  context_command_->setRenderTarget(swap_chain_buffer->getDescriptor());
}
void GraphicsContext::setGraphicsBindLayout(
    const std::shared_ptr<Pipeline::BindLayout> &bind_layout) {
  graphics_layout_ = bind_layout;
  context_command_->setGraphicsBindSignature(bind_layout->bind_signature_);
}
void GraphicsContext::bindGraphicsResource(
    const std::shared_ptr<Resource::Resource> &resource,
    unsigned int usage_index, unsigned int slot_index, BindType bind_type,
    bool direct_bind) {
  context_command_->bindGraphicsResource(resource, usage_index, slot_index,
                                         bind_type, direct_bind);
}
void GraphicsContext::bindGraphicsResource(
    const std::shared_ptr<Resource::Resource> &resource,
    unsigned int slot_index, unsigned int usage_index) {
  if (!graphics_layout_) {
    throw std::exception(
        "Need to set graphics bind layout first in this context");
  }
  bool direct_bind = graphics_layout_->isDirectBind(slot_index);
  auto type = graphics_layout_->getFirstBindType(slot_index);
  bindGraphicsResource(resource, usage_index, slot_index, type, direct_bind);
}
void GraphicsContext::bindGraphicsResource(
    const std::shared_ptr<Resource::Resource> &resource,
    const std::string &slot_name, unsigned int usage_index) {
  if (!graphics_layout_) {
    throw std::exception(
        "Need to set graphics bind layout first in this context");
  }
  unsigned int slot_index = graphics_layout_->getSlotIndex(slot_name);
  bindGraphicsResource(resource, slot_index, usage_index);
}

void GraphicsContext::bindGraphicsSampler(
    const std::shared_ptr<Sampler::Sampler> &sampler, unsigned int slot_index) {
  if (!graphics_layout_) {
    throw std::exception(
        "Need to set graphics bind layout first in this context");
  }
  auto type = graphics_layout_->getFirstBindType(slot_index);
  if (type != BindType::BIND_TYPE_SIT_SAMPLER) {
    throw std::exception("Error slot index, the slot should be for sampler");
  }
  context_command_->bindGraphicsSampler(sampler, 0, slot_index);
}

void GraphicsContext::bindGraphicsSampler(
    const std::shared_ptr<Sampler::Sampler> &sampler,
    const std::string &slot_name) {
  if (!graphics_layout_) {
    throw std::exception(
        "Need to set graphics bind layout first in this context");
  }
  unsigned int slot_index = graphics_layout_->getSlotIndex(slot_name);
  bindGraphicsSampler(sampler, slot_index);
}

void GraphicsContext::bindGraphicsSamplers(
    const std::shared_ptr<Sampler::Sampler> &sampler_group,
    unsigned int slot_index, unsigned int start_index) {
  if (!graphics_layout_) {
    throw std::exception(
        "Need to set graphics bind layout first in this context");
  }
  auto type = graphics_layout_->getFirstBindType(slot_index);
  if (type != BindType::BIND_TYPE_SIT_SAMPLER) {
    throw std::exception("Error slot index, the slot should be for sampler");
  }
  context_command_->bindGraphicsSampler(sampler_group, start_index, slot_index);
}

void GraphicsContext::bindGraphicsSamplers(
    const std::shared_ptr<Sampler::Sampler> &sampler_group,
    const std::string &slot_name, unsigned int start_index) {
  if (!graphics_layout_) {
    throw std::exception(
        "Need to set graphics bind layout first in this context");
  }
  unsigned int slot_index = graphics_layout_->getSlotIndex(slot_name);
  bindGraphicsSamplers(sampler_group, slot_index, start_index);
}

void GraphicsContext::setSwapChainToPresetState(
    const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer) {
  updateContextResourceState(swap_chain_buffer,
                             ResourceState::RESOURCE_STATE_PRESENT, false, 0);
}

} // namespace Context
} // namespace Renderer
} // namespace CHCEngine