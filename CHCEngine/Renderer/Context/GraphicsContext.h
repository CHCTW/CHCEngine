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
class Texture;
} // namespace Resource
namespace Pipeline {
class BindLayout;
}
namespace Sampler {
class Sampler;
class SamplerGroup;
} // namespace Sampler
namespace Context {
struct RenderTargetSetting {
  const std::shared_ptr<Resource::Texture> &texture_;
  uint32_t usage_indices[8] = {0};
  uint32_t usage_size = 1;
};
static const std::shared_ptr<Resource::Texture> dummy_depth_texture = nullptr;
class GraphicsContext : public ComputeContext {
protected:
  friend class Renderer;
  void bindGraphicsResource(const std::shared_ptr<Resource::Resource> &resource,
                            unsigned int usage_index, unsigned int slot_index,
                            BindType bind_type, bool direct_bind);
  void bindGraphicsResource(const std::shared_ptr<Resource::Resource> &resource,
                            unsigned int slot_index,
                            unsigned int usage_index = 0);
  void flushGraphicsBindings();
  void updateRenderTargetTextureState(
      const std::shared_ptr<Resource::Texture> &texture, uint32_t usage_index);
  void updateDepthStencilTextureState(
      const std::shared_ptr<Resource::Texture> &texture, uint32_t usage_index,
      bool write);

public:
  ~GraphicsContext() {
    // waitRecordingDone();
  }
  GraphicsContext(CommandType type,
                  std::shared_ptr<ContextCommand> context_command,
                  std::weak_ptr<ContextPoolBase> pool)
      : ComputeContext(type, context_command, pool) {}
  void clearRenderTarget(
      const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer,
      Color color);
  void
  clearRenderTarget(const std::shared_ptr<Resource::Texture> &render_target,
                    Color color, uint32_t render_target_usage_index = 0);
  void
  clearRenderTarget(const std::shared_ptr<Resource::Texture> &render_target,
                    uint32_t render_target_usage_index = 0);

  void
  clearDepth(const std::shared_ptr<Resource::Texture> &depth_stencil_texture,
             float value, uint32_t depth_stencil_usage_index = 0);
  void
  clearStencil(const std::shared_ptr<Resource::Texture> &depth_stencil_texture,
               uint8_t value, uint32_t depth_stencil_usage_index = 0);
  void clearDepthStencil(
      const std::shared_ptr<Resource::Texture> &depth_stencil_texture,
      float depth_value, uint8_t stencil_value,
      uint32_t depth_stencil_usage_index = 0);
  // use the default clear value, faster, but need to predefine
  void clearDepthStencil(
      const std::shared_ptr<Resource::Texture> &depth_stencil_texture,
      uint32_t depth_stencil_usage_index = 0);
  void drawInstanced(unsigned int vertex_count, unsigned int instance_count = 1,
                     unsigned int start_vertex_location = 0,
                     unsigned int start_instance_location = 0);
  void setViewport(const Pipeline::Viewport &viewport);
  void setScissor(const Pipeline::Scissor &scissor);
  void setVertexBuffers(
      const std::vector<std::shared_ptr<Resource::Buffer>> &buffers);
  template <typename... T> void setVertexBuffers(T &&... buffers) {
    setVertexBuffers({std::forward<T>(buffers)...});
  }
  void setPrimitiveTopology(PrimitiveTopology topology);
  void setRenderTarget(
      const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer);
  void setRenderTarget(
      const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer,
      const std::shared_ptr<Resource::Texture> &depth_texture,
      uint32_t depth_stencil_index = 0, bool write = true);
  void setRenderTarget(
      const std::vector<RenderTargetSetting> &render_target_settings,
      const std::shared_ptr<Resource::Texture> &depth_texture =
          dummy_depth_texture,
      uint32_t depth_stencil_index = 0, bool write = true);

  // unfinished, should also  bind the resource in the bind layout ,will add it
  // later
  void setGraphicsBindLayout(
      const std::shared_ptr<Pipeline::BindLayout> &bind_layout);
  void bindGraphicsResource(const std::shared_ptr<Resource::Resource> &resource,
                            const std::string &slot_name,
                            unsigned int usage_index = 0);
  void bindGraphicsConstants(const void *data, uint32_t num_32bit_constant,
                             const std::string &slot_name,
                             uint32_t constant_offset = 0);
  void bindGraphicsConstants(const void *data, uint32_t num_32bit_constant,
                             uint32_t slot_index, uint32_t constant_offset = 0);

  void bindGraphicsSampler(const std::shared_ptr<Sampler::Sampler> &resource,
                           unsigned int slot_index);
  void bindGraphicsSampler(const std::shared_ptr<Sampler::Sampler> &sampler,
                           const std::string &slot_name);
  void
  bindGraphicsSamplers(const std::shared_ptr<Sampler::Sampler> &resource_group,
                       unsigned int slot_index, unsigned int start_index = 0);
  void
  bindGraphicsSamplers(const std::shared_ptr<Sampler::Sampler> &sampler_group,
                       const std::string &slot_name,
                       unsigned int start_index = 0);
  void setSwapChainToPresetState(
      const std::shared_ptr<Resource::SwapChainBuffer> &swap_chain_buffer);
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine