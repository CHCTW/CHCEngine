#include "../ClassName.h"

#include "../Pipeline/Pipeline.h"
#include "../Resource/SwapChainBuffer.h"
#include "../Resource/Texture.h"
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
  context_command_->clearRenderTarget(swap_chain_buffer->getDescriptor(), c);
}
void GraphicsContext::clearRenderTarget(
    const std::shared_ptr<Resource::Texture> &render_target, Color color,
    uint32_t render_target_usage_index) {

  float c[4] = {color.r, color.g, color.b, color.a};
  updateRenderTargetTextureState(render_target, render_target_usage_index);
  flushBarriers();
  context_command_->clearRenderTarget(
      render_target->getRenderTargetDescriptor(render_target_usage_index), c);
}
void GraphicsContext::clearRenderTarget(
    const std::shared_ptr<Resource::Texture> &render_target,
    uint32_t render_target_usage_index) {
  updateRenderTargetTextureState(render_target, render_target_usage_index);
  flushBarriers();

  const auto &inf = render_target->getTextureInformation();
  if (inf.default_clear_value_.type_ !=
      DefaultClearValueType::DEFAULT_CLEAR_VALUE_TYPE_COLOR) {
    std::string error =
        "Didn't setup default render target clear value, Texture Name: ";
    error += render_target->getInformation().name_;
    throw std::exception(error.c_str());
  }
  float c[4] = {
      inf.default_clear_value_.color_.r, inf.default_clear_value_.color_.g,
      inf.default_clear_value_.color_.b, inf.default_clear_value_.color_.a};
  context_command_->clearRenderTarget(
      render_target->getRenderTargetDescriptor(render_target_usage_index), c);
}
void GraphicsContext::clearDepth(
    const std::shared_ptr<Resource::Texture> &depth_stencil_texture,
    float value, uint32_t depth_stencil_usage_index) {
  updateDepthStencilTextureState(depth_stencil_texture,
                                 depth_stencil_usage_index, true);
  flushBarriers();
  context_command_->clearDepthStencil(
      depth_stencil_texture->getDepthStencilDescriptor(
          depth_stencil_usage_index),
      value, 0, true, false);
}
void GraphicsContext::clearStencil(
    const std::shared_ptr<Resource::Texture> &depth_stencil_texture,
    uint8_t value, uint32_t depth_stencil_usage_index) {
  updateDepthStencilTextureState(depth_stencil_texture,
                                 depth_stencil_usage_index, true);
  flushBarriers();
  context_command_->clearDepthStencil(
      depth_stencil_texture->getDepthStencilDescriptor(
          depth_stencil_usage_index),
      0, value, false, true);
}
void GraphicsContext::clearDepthStencil(
    const std::shared_ptr<Resource::Texture> &depth_stencil_texture,
    float depth_value, uint8_t stencil_value,
    uint32_t depth_stencil_usage_index) {
  updateDepthStencilTextureState(depth_stencil_texture,
                                 depth_stencil_usage_index, true);
  flushBarriers();
  context_command_->clearDepthStencil(
      depth_stencil_texture->getDepthStencilDescriptor(
          depth_stencil_usage_index),
      depth_value, stencil_value, true, true);
}
void GraphicsContext::clearDepthStencil(
    const std::shared_ptr<Resource::Texture> &depth_stencil_texture,
    uint32_t depth_stencil_usage_index) {
  updateDepthStencilTextureState(depth_stencil_texture,
                                 depth_stencil_usage_index, true);
  flushBarriers();
  const auto &inf = depth_stencil_texture->getTextureInformation();
  auto format = convertToDataFormat(
      depth_stencil_texture->depth_stencil_usages_[0].data_format_);
  if (inf.default_clear_value_.type_ !=
      DefaultClearValueType::DEFAULT_CLEAR_VALUE_TYPE_DEPTH_STENCIL) {
    std::string error =
        "Didn't setup default depth stencil clear value, Texture Name: ";
    error += depth_stencil_texture->getInformation().name_;
    throw std::exception(error.c_str());
  }
  context_command_->clearDepthStencil(
      depth_stencil_texture->getDepthStencilDescriptor(
          depth_stencil_usage_index),
      inf.default_clear_value_.depth_stencil_value_.Depth,
      inf.default_clear_value_.depth_stencil_value_.Stencil,
      hasDepthFormat(format), hasStencilFormat(format));
}
void GraphicsContext::drawInstanced(unsigned int vertex_count,
                                    unsigned int instance_count,
                                    unsigned int start_vertex_location,
                                    unsigned int start_instance_location) {

  flushBarriers();
  flushGraphicsBindings();
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
  for (auto &buffer : buffers) {
    updateContextResourceState(
        buffer, ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, false,
        0);
  }
  context_command_->setVertexBuffers(buffers);
}
void GraphicsContext::setPrimitiveTopology(PrimitiveTopology topology) {
  context_command_->setTopology(topology);
}
void GraphicsContext::setRenderTarget(
    const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer) {
  updateContextResourceState(
      swap_chain_buffer, ResourceState::RESOURCE_STATE_RENDER_TARGET, false, 0);
  context_command_->setRenderTarget(swap_chain_buffer->getDescriptor());
}
void GraphicsContext::setRenderTarget(
    const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer,
    const std::shared_ptr<Resource::Texture> &depth_texture,
    uint32_t depth_stencil_index, bool write) {
  updateContextResourceState(
      swap_chain_buffer, ResourceState::RESOURCE_STATE_RENDER_TARGET, false, 0);
  updateDepthStencilTextureState(depth_texture, depth_stencil_index, write);
  context_command_->setRenderTarget(
      &swap_chain_buffer->getDescriptor(), 1,
      &depth_texture->getDepthStencilDescriptor(depth_stencil_index));
}
void GraphicsContext::setRenderTarget(
    const std::vector<RenderTargetSetting> &render_target_settings,
    const std::shared_ptr<Resource::Texture> &depth_texture,
    uint32_t depth_stencil_index, bool write) {
  uint32_t total = 0;
  CPUDescriptorHandle handles[8];
  for (const auto &r : render_target_settings) {
    for (uint32_t i = 0; i < r.usage_size; ++i) {
      updateRenderTargetTextureState(r.texture_, r.usage_indices[i]);
      handles[i] = r.texture_->getRenderTargetDescriptor(r.usage_indices[i]);
      ++total;
      if (total >= 8u) {
        throw std::exception("Set too many render target, It only support 8 "
                             "render targets maximum!");
      }
    }
  }
  if (depth_texture) {
    updateDepthStencilTextureState(depth_texture, depth_stencil_index, write);
    context_command_->setRenderTarget(
        handles, total,
        &depth_texture->getDepthStencilDescriptor(depth_stencil_index));
  } else {
    context_command_->setRenderTarget(handles, total, nullptr);
  }
  // context_command_->setRenderTarget(handles,total,)
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
  updateBindingResrouceState(resource, slot_index, 0, usage_index,
                             graphics_layout_);
  // so... before binding the resource, it request the resource
  // to be the right, state....
  //  approach: all binding are deferred before the real
  // draw call or dispatch...
  // there will be flush bindings so the debug layer won't cry

  // putting flush barrier here to flush before real bind recourse
  // just as temporary code
  // btw the volatile range flag will cause not track the resource
  // flushBarriers();

  graphics_bind_descriptors_.push(
      {resource.get(), usage_index, slot_index, type, direct_bind});
  /*bindGraphicsResource(resource, usage_index, slot_index, type,
   * direct_bind);*/
}
void GraphicsContext::flushGraphicsBindings() {
  while (graphics_bind_descriptors_.size()) {
    auto &bind = graphics_bind_descriptors_.front();
    context_command_->bindGraphicsResource(bind.resource_, bind.usage_index_,
                                           bind.slot_index_, bind.bind_type_,
                                           bind.direct_bind_);
    graphics_bind_descriptors_.pop();
  }
}
void GraphicsContext::updateRenderTargetTextureState(
    const std::shared_ptr<Resource::Texture> &texture, uint32_t usage_index) {
  const auto &sub_resource_list =
      texture->getRenderTargetSubResourceIndices(usage_index);
  for (auto i : sub_resource_list) {
    updateContextResourceState(
        texture, ResourceState::RESOURCE_STATE_RENDER_TARGET, false, i);
  }
}
void GraphicsContext::updateDepthStencilTextureState(
    const std::shared_ptr<Resource::Texture> &texture, uint32_t usage_index,
    bool write) {
  const auto &sub_resource_list =
      texture->getDepthStencilSubResourceIndices(usage_index);
  auto state = ResourceState::RESOURCE_STATE_DEPTH_READ;
  if (write)
    state = ResourceState::RESOURCE_STATE_DEPTH_WRITE;
  for (auto i : sub_resource_list) {
    updateContextResourceState(texture, state, false, i);
  }
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

void GraphicsContext::bindGraphicsConstants(const void *data,
                                            uint32_t num_32bit_constant,
                                            const std::string &slot_name,
                                            uint32_t constant_offset) {
  if (!graphics_layout_) {
    throw std::exception(
        "Need to set graphics bind layout first in this context");
  }
  unsigned int slot_index = graphics_layout_->getSlotIndex(slot_name);
  bindGraphicsConstants(data, num_32bit_constant, slot_index, constant_offset);
}

void GraphicsContext::bindGraphicsConstants(const void *data,
                                            uint32_t num_32bit_constant,
                                            uint32_t slot_index,
                                            uint32_t constant_offset) {
  if (!graphics_layout_) {
    throw std::exception(
        "Need to set graphics bind layout first in this context");
  }
  auto total =
      graphics_layout_->getBindSlot(slot_index).formats_[0].resource_count_;
  if (num_32bit_constant + constant_offset > total) {
    throw std::exception("Bind Graphics Constants bind too many constants or "
                         "have too large offset");
  }
  context_command_->bindGraphicsConstants(data, slot_index, num_32bit_constant,
                                          constant_offset);
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