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
#include "Resource/ResourcePool.h"
#include "Resource/SwapChainBuffer.h"
#include "Pipeline/DynamicBuffer.h"


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

  std::shared_ptr<Pipeline::DynamicBuffer> dynamic_upload_buffer_;


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
  std::shared_ptr<ContextFence>
  submitContexts(std::shared_ptr<ContextFence> fence,
                 std::vector<std::shared_ptr<GraphicsContext>> &&contexts);

  template <class... ContextPTRClass>
  std::shared_ptr<ContextFence>
  submitContexts(std::shared_ptr<ContextFence> fence,
                 ContextPTRClass &&... context_list) {
    return submitContexts(fence,
                          {std::forward<ContextPTRClass>(context_list)...});
  }

  std::shared_ptr<Resource::SwapChainBuffer>
  getSwapChainBuffer(int swap_chain_index) {
    return swap_chain_buffers_[swap_chain_index];
  }
  std::shared_ptr<ContextFence> getContextFence();
  /* attribute are <sematic,format> the sematic name should be the same
   in the hlsl file, don't use the SV series*/
  std::shared_ptr<Resource::Buffer> getVertexBuffer(
      unsigned int vertex_count,
      const std::vector<std::pair<std::string, DataFormat>> &attributes,
      ResourceState initial_state = ResourceState::RESOURCE_STATE_COPY_DEST,
      Resource::ResourceUsage usage =
          Resource::ResourceUsage::RESOURCE_USAGE_STATIC) {
    return resource_pool_->getVertexBuffer(vertex_count, attributes,
                                           initial_state, usage);
  }

  std::shared_ptr<Resource::Buffer> getIndexBuffer(
      unsigned int index_count,
      IndexFormat index_format = IndexFormat::INDEX_FORMAT_32_UINT,
      ResourceState initial_state = ResourceState::RESOURCE_STATE_COPY_DEST,
      Resource::ResourceUsage usage =
          Resource::ResourceUsage::RESOURCE_USAGE_STATIC) {
    return resource_pool_->getIndexBuffer(index_count, index_format,
                                          initial_state, usage);
  }
  // shader visiblity should be the same in the same slot, but it's find.
  // just going to set as all vis in rootsignature, but it's probally be nice to
  // have the same visiblity
  std::shared_ptr<Pipeline::BindLayout>
  getBindLayout(const std::vector<Pipeline::BindSlot> &bind_layout);
  std::shared_ptr<Pipeline::Pipeline> getGraphicsPipeline(
      const ShaderSet &shader_set,
      const std::vector<Resource::Attributes> &attributes,
      std::shared_ptr<BindLayout> bind_layout,
      const RenderTargetSetup &render_setup = Pipeline::default_render_setup_,
      const DepthStencilSetup &depth = Pipeline::default_depth_stencil_setup_,
      const Rasterizer &rasterizer = Pipeline::default_rasterizer_,
      PrimitiveTopologyType primitive =
          PrimitiveTopologyType::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
      const Pipeline::SampleSetup &sample = Pipeline::defualt_sample_setup);
};

} // namespace Renderer
} // namespace CHCEngine