#pragma once
#include <wrl/client.h>

#include <chrono>
#include <functional>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "../Window/Window.h"
#include "ClassName.h"
#include "CommandQueue.h"
#include "Context/ContextPool.h"
#include "Context/ContextQueue.h"
#include "Context/FencePool.h"
#include "Context/GraphicsContext.h"
#include "DescriptorHeap.h"
#include "Pipeline/BindLayout.h"
#include "Pipeline/Pipeline.h"
#include "Resource/DynamicBuffer.h"
#include "Resource/ResourcePool.h"
#include "Resource/SwapChainBuffer.h"
#include "Sampler/Sampler.h"
#include "Sampler/SamplerGroup.h"

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
static unsigned int CBV_SRV_UAV_HEAP_SIZE = 10000;
static unsigned int RTV_HEAP_SIZE = 5000;
static unsigned int DSV_HEAP_SIZE = 5000;
static unsigned int SAMPLER_HEAP_SIZE = 1000;
class Renderer;
using RenderLoopCallback = std::function<void(
    Renderer &renderer,
    std::chrono::duration<long long, std::nano> const &delta,
    unsigned int swap_chain_index, unsigned long long const &frame)>;
using Context::ComputeContext;
using Context::ContextFence;
using Context::CopyContext;
using Context::GraphicsContext;
using Pipeline::BindLayout;
using Pipeline::BlendState;
using Pipeline::DepthStencilSetup;
using Pipeline::Rasterizer;
using Pipeline::RenderTargetSetup;
using Pipeline::ShaderSet;
class Renderer {
private:
  Window::Window *window_;
  std::ostream *output_stream_;
  ComPtr<Device> device_;
  ComPtr<Factory> factory_;
  ComPtr<SwapChain> swap_chain_;
  void checkSupportFeatures();
  void createCommandQueues();
  void createDescriptorHeap();
  void createResourcePool();
  void createContexts();
  // going to be referenced by other resources, so use shared_ptr
  // std::unordered_map<CommandType, ComPtr<CommandQueue>> command_queues_;
  // a static descriptor heaps, used to store all static resources desc
  // not going to use for shader binding, going to copy the desc to the other
  // heaps (srv_cbv_uav,sampler)

  std::unordered_map<CommandType, std::shared_ptr<Context::ContextQueue>>
      context_queues_;
  std::unordered_map<DescriptorType, std::shared_ptr<DescriptorHeap>>
      static_heaps_;
  std::shared_ptr<DescriptorHeap> shader_visible_resource_heap_;
  std::shared_ptr<DescriptorHeap> shader_visible_sampler_heap_;

  std::unordered_map<DescriptorType, unsigned int> heap_sizes_;
  std::shared_ptr<DescriptorRange> swap_chain_descriptor_range_;

  std::shared_ptr<Resource::ResourcePool> resource_pool_;
  unsigned int swap_chain_count_;
  std::vector<std::shared_ptr<Resource::SwapChainBuffer>> swap_chain_buffers_;
  std::shared_ptr<Context::ContextPool<Context::GraphicsContext>>
      graphics_pool_;
  std::shared_ptr<Context::ContextPool<Context::ComputeContext>> compute_pool_;
  std::shared_ptr<Context::ContextPool<Context::CopyContext>> copy_pool_;

  std::vector<std::shared_ptr<Context::ContextFence>> swap_chain_present_fence_;

  //  loop area, just like window, have backbuffer index,time, frame
  void loop();
  void calculateDelta();
  std::thread loop_thread_;
  unsigned int swap_chain_index_;
  unsigned long long frame_;
  std::chrono::duration<long long, std::nano> delta_;
  std::chrono::time_point<std::chrono::high_resolution_clock> previous_time_;

  std::unordered_map<std::string, RenderLoopCallback> loop_callbacks;
  std::unordered_set<std::string> loop_callback_names_;
  std::mutex loop_callback_names_mutex_;
  std::deque<std::tuple<std::string, RenderLoopCallback>>
      loop_callback_adding_queue_;
  std::vector<std::string> loop_remove_names_;
  std::mutex loop_queue_mutex_;
  std::mutex loop_remove_mutex_;

  bool checkLoopCallbackAndAdd(std::string name);
  void addLoopCallbackFromQueue();
  void removeLoopCallbackFromNames();
  void loopCalls();

  std::shared_ptr<Resource::DynamicBuffer> dynamic_upload_buffer_;

  // fence pool should be at the bottom of this class, in desc, this will
  // wait all the fence thus no resouce will be release while looping
  // however, fence may still need to record the using resource
  std::shared_ptr<Context::FencePool> fence_pool_;

public:
  Renderer();
  bool initializeDevice();
  void setSwapChain(Window::Window &window, unsigned int swap_chain_count = 3);
  void presentSwapChain();
  void waitUntilWindowClose();
  bool addLoopCallback(std::string name, RenderLoopCallback callback);
  bool checkLoopCallbackNameExist(std::string name);
  void removeLoopCallback(std::string name);
  std::shared_ptr<GraphicsContext>
  getGraphicsContext(std::function<void(GraphicsContext *)> callback = 0,
                     bool async = true);
  std::shared_ptr<ComputeContext>
  getComputeContext(std::function<void(ComputeContext *)> callback = 0,
                    bool async = true);

  std::shared_ptr<CopyContext>
  getCopyContext(std::function<void(CopyContext *)> callback = 0,
                 bool async = true);
  const std::shared_ptr<ContextFence> &submitContexts(
      const std::shared_ptr<ContextFence> &fence,
      const std::vector<std::shared_ptr<Context::Context>> &contexts);

  std::shared_ptr<ContextFence> submitContexts(
      const std::vector<std::shared_ptr<Context::Context>> &contexts);

  void waitFenceValue(const std::shared_ptr<ContextFence> &fence,
                      uint64_t wait_value, CommandType type);

  std::shared_ptr<ContextFence> waitFenceValueSubmitContexts(
      const std::shared_ptr<ContextFence> &fence, uint64_t wait_value,
      const std::vector<std::shared_ptr<Context::Context>> &contexts);

  const std::shared_ptr<ContextFence> &waitFenceValueSubmitContexts(
      const std::shared_ptr<ContextFence> &wait_fence, uint64_t wait_value,
      const std::shared_ptr<ContextFence> &fence,
      const std::vector<std::shared_ptr<Context::Context>> &contexts);
  // usually work for the fence that is already submitted contexts
  std::shared_ptr<ContextFence> waitFenceSubmitContexts(
      const std::shared_ptr<ContextFence> &fence,
      const std::vector<std::shared_ptr<Context::Context>> &contexts) {
    auto state = fence->getStateAndExpectedValue();
    // the commands already finished executing, doesn't need to wait
    if (state.first == Context::FenceState::FENCE_STATE_IDLE) {
      return submitContexts(contexts);
    }
    return waitFenceValueSubmitContexts(fence, state.second, contexts);
  }
  const std::shared_ptr<ContextFence> &waitFenceSubmitContexts(
      const std::shared_ptr<ContextFence> &wait_fence,
      const std::shared_ptr<ContextFence> &fence,
      const std::vector<std::shared_ptr<Context::Context>> &contexts) {
    auto state = wait_fence->getStateAndExpectedValue();
    // the commands already finished executing, doesn't need to wait
    if (state.first == Context::FenceState::FENCE_STATE_IDLE) {
      return submitContexts(fence, contexts);
    }
    return waitFenceValueSubmitContexts(wait_fence, state.second, fence,
                                        contexts);
  }

  /*template <std::shared_ptr<ContextFence> &,class... ContextPTRClass>
  std::shared_ptr<ContextFence> &
  submitContexts(const std::shared_ptr<ContextFence> &fence,
                 ContextPTRClass &&... context_list) {
    return submitContexts(fence,
                          {std::forward<ContextPTRClass>(context_list)...});
  }

  template <class... ContextPTRClass>
  std::shared_ptr<ContextFence>
  submitContexts(ContextPTRClass &&... context_list) {
    return submitContexts({std::forward<ContextPTRClass>(context_list)...});
  }*/

  const std::shared_ptr<Resource::SwapChainBuffer> &
  getSwapChainBuffer(int swap_chain_index) {
    return swap_chain_buffers_[swap_chain_index];
  }
  std::shared_ptr<ContextFence> getContextFence();
  /* attribute are <sematic,format> the sematic name should be the same
   in the hlsl file, don't use the SV series*/
  std::shared_ptr<Resource::Buffer> getVertexBuffer(
      unsigned int vertex_count,
      const std::vector<std::pair<std::string, DataFormat>> &attributes,
      ResourceState initial_state = ResourceState::RESOURCE_STATE_COMMON,
      Resource::ResourceUpdateType update_type =
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC) {
    return resource_pool_->getVertexBuffer(vertex_count, attributes,
                                           initial_state, update_type);
  }

  std::shared_ptr<Resource::Buffer> getIndexBuffer(
      unsigned int index_count,
      IndexFormat index_format = IndexFormat::INDEX_FORMAT_32_UINT,
      ResourceState initial_state = ResourceState::RESOURCE_STATE_COMMON,
      Resource::ResourceUpdateType update_type =
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC) {
    return resource_pool_->getIndexBuffer(index_count, index_format,
                                          initial_state, update_type);
  }

  std::shared_ptr<Resource::Buffer> getBuffer(
      unsigned int element_count, unsigned int element_byte_size,
      const std::vector<BufferUsage> &usages,
      ResourceState initial_state = ResourceState::RESOURCE_STATE_COMMON,
      Resource::ResourceUpdateType update_type =
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC,
      const std::vector<std::pair<std::string, DataFormat>> &attributes = {},
      IndexFormat index_format = IndexFormat::INDEX_FORMAT_NONE) {

    return resource_pool_->getBuffer(element_count, element_byte_size, usages,
                                     attributes, index_format, initial_state,
                                     update_type);
  }
  std::shared_ptr<Resource::Texture> getTexture(
      TextureType texture_type, RawFormat raw_format, unsigned long long width,
      unsigned int height, unsigned int depth, unsigned int mip_levels,
      const std::vector<TextureUsage> &usages,
      const std::vector<RenderTargetUsage> &render_target_usages =
          empty_render_target_usage,
      const std::vector<DepthStencilUsage> &depth_stencil_usages =
          empty_depth_stencil_usage,
      const DefaultClearValue &clear_value = default_depth_stencil_clear_value,
      ResourceState initial_state = ResourceState::RESOURCE_STATE_COMMON,
      Resource::ResourceUpdateType update_type =
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC) {
    return resource_pool_->getTexture(
        texture_type, raw_format, width, height, depth, mip_levels, usages,
        render_target_usages, depth_stencil_usages, clear_value, initial_state,
        update_type);
  }

  // shader visiblity should be the same in the same slot, but it's fine.
  // just going to set as all vis in rootsignature, but it's probally be nice to
  // have the same visiblity
  std::shared_ptr<Pipeline::BindLayout>
  getBindLayout(const std::vector<Pipeline::BindSlot> &bind_layout);

  // a simple version for binding formatn, this one means every bind resrouce
  // takes it's own slot, except unbound or array resources
  std::shared_ptr<Pipeline::BindLayout>
  getBindLayout(const std::vector<Pipeline::BindFormat> &bind_layout);

  std::shared_ptr<Pipeline::Pipeline> getGraphicsPipeline(
      const ShaderSet &shader_set,
      const std::vector<Resource::Attributes> &attributes,
      const std::shared_ptr<BindLayout> &bind_layout,
      const RenderTargetSetup &render_setup = Pipeline::default_render_setup_,
      const DepthStencilSetup &depth = Pipeline::default_depth_stencil_setup_,
      const Rasterizer &rasterizer = Pipeline::default_rasterizer_,
      PrimitiveTopologyType primitive =
          PrimitiveTopologyType::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
      const Pipeline::SampleSetup &sample = Pipeline::defualt_sample_setup);
  std::shared_ptr<Resource::ResourceGroup> getResourceGroup(unsigned int size) {
    return resource_pool_->getResrouceGroup(size);
  }
  std::shared_ptr<Sampler::Sampler>
  getSampler(const Sampler::SamplerInformation &sampler_inf);
  std::shared_ptr<Sampler::SamplerGroup> getSamplerGroup(unsigned int size);
  std::shared_ptr<Pipeline::Pipeline>
  getComputePipeline(const Pipeline::Shader &shader,
                     const std::shared_ptr<BindLayout> &bind_layout);
  uint32_t getSwapChainIndex();
};

} // namespace Renderer
} // namespace CHCEngine