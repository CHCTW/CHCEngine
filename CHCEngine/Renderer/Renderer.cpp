#include "ClassName.h"

#include "Renderer.h"

#include <dxgi1_6.h>
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <d3d12.h>
#include <d3dx12.h>
#include <glfw/glfw3native.h>
#include <wrl/client.h>

#include <iostream>
#include <magic_enum.hpp>
#include <memory>

#include "Context/Command.h"
#include "D3D12Convert.h"
#include "D3D12Utilities.hpp"
#include "Utilities.hpp"
static D3D12_FEATURE_DATA_ROOT_SIGNATURE root_signature_feature = {};
using CHCEngine::Renderer::Context::GraphicsContext;
using dur = std::chrono::duration<long long, std::nano>;
namespace CHCEngine {
namespace Renderer {
static const GUID D3D12ExperimentalShaderModelsID = {
    0x76f5573e,
    0xf13a,
    0x40f5,
    {0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f}};
static const unsigned int root_sigature_size_limits = 64;
void Renderer::checkSupportFeatures() {

  root_signature_feature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

  if (FAILED(device_->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
                                          &root_signature_feature,
                                          sizeof(root_signature_feature)))) {
    root_signature_feature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
  }
  D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {D3D_SHADER_MODEL_6_5};
  if (FAILED(device_->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL,
                                          &shaderModel, sizeof(shaderModel))) ||
      (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5)) {
    *output_stream_ << "ERROR: Shader Model 6.5 is not supported\n";
    throw std::exception("Shader Model 6.5 is not supported");
  }

  //  need to wait for driver version over 450.56
  //  https://developer.nvidia.com/directx
  // can download here

  D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
  if (FAILED(device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7,
                                          &features, sizeof(features))) ||
      (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)) {
    *output_stream_ << "ERROR: Mesh Shaders aren't supported!\n";
    throw std::exception("Mesh Shaders aren't supported!");
  }

  D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};
  if (FAILED(device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
                                          &featureSupportData,
                                          sizeof(featureSupportData))) ||
      (featureSupportData.RaytracingTier ==
       D3D12_RAYTRACING_TIER_NOT_SUPPORTED)) {
    *output_stream_ << "ERROR: Ray tracing aren't supported!\n";
    throw std::exception("Mesh Shaders aren't supported!");
  }
}

void Renderer::createCommandQueues() {
  for (int i = 0; i != static_cast<int>(CommandType::COMMAND_TYPE_QUEUE_BOUND);
       ++i) {
    CommandType type = static_cast<CommandType>(i);
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = convertToD3D12CommandType(type);
    ComPtr<CommandQueue> queue;
    ThrowIfFailed(
        device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)));
    std::string name(magic_enum::enum_name(type));
    NAME_D3D12_OBJECT_STRING(queue, name);
    context_queues_[type] =
        std::make_shared<Context::ContextQueue>(queue, type);
  }
}
void Renderer::createDescriptorHeap() {
  for (int i = 0; i != static_cast<int>(DescriptorType::DESCRIPTOR_TYPE_COUNT);
       ++i) {
    DescriptorType type = static_cast<DescriptorType>(i);
    std::string name(magic_enum::enum_name(type));
    name += "_STATIC";
    static_heaps_[type] = std::make_shared<DescriptorHeap>(
        device_, type, heap_sizes_[type], name, true);
  }
  std::string name("SHADER_VISIBLE_RESOURCE_DESCRIPTOR_HEAP");
  shader_visible_resource_heap_ = std::make_shared<DescriptorHeap>(
      device_, DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV,
      heap_sizes_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV], name, true);
  std::string name_sampler("SHADER_VISIBLE_SAMPLER_DESCRIPTOR_HEAP");
  shader_visible_sampler_heap_ = std::make_shared<DescriptorHeap>(
      device_, DescriptorType::DESCRIPTOR_TYPE_SAMPLER,
      heap_sizes_[DescriptorType::DESCRIPTOR_TYPE_SAMPLER], name_sampler, true);
}
void Renderer::createResourcePool() {
  resource_pool_ = std::make_shared<Resource::ResourcePool>(
      device_, static_heaps_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV],
      static_heaps_[DescriptorType::DESCRIPTOR_TYPE_RTV],
      static_heaps_[DescriptorType::DESCRIPTOR_TYPE_DSV]);
}
void Renderer::createContexts() {
  graphics_pool_ =
      std::make_shared<Context::ContextPool<Context::GraphicsContext>>(
          device_, CommandType::COMMAND_TYPE_GRAPHICS);
  compute_pool_ =
      std::make_shared<Context::ContextPool<Context::ComputeContext>>(
          device_, CommandType::COMMAND_TYPE_COMPUTE);
  copy_pool_ = std::make_shared<Context::ContextPool<Context::CopyContext>>(
      device_, CommandType::COMMAND_TYPE_COMPUTE);
  auto grpahics_context = graphics_pool_->getContext([](GraphicsContext *t) {
    std::cout << "graphics wait" << std::endl;
    std::cout << "graphics wait end" << std::endl;
  });

  fence_pool_ = std::make_shared<Context::FencePool>(device_);
  auto fence = fence_pool_->getContextFence();

  std::vector<std::shared_ptr<Context::Context>> t = {grpahics_context};
  context_queues_[CommandType::COMMAND_TYPE_GRAPHICS]->SubmitContextCommand(
      t, fence);

  /*std::vector<std::shared_ptr<Context::Context>> ct = {compute_context};
  context_queues_[CommandType::COMMAND_TYPE_COMPUTE]->SubmitContextCommand(
      ct, fence);*/
}
void Renderer::loop() {
  previous_time_ = std::chrono::high_resolution_clock::now();
  while (!window_->shouldWindowClose()) {
    calculateDelta();
    swap_chain_index_ = swap_chain_->GetCurrentBackBufferIndex();
    ++frame_;
    /*std::cout << swap_chain_index_ << "   " << delta_.count() / 1000000000.0
              << "  " << frame_ << std::endl;
    presentSwapChain();*/
    addLoopCallbackFromQueue();
    removeLoopCallbackFromNames();
    // loop call here
    loopCalls();
  }
}
void Renderer::calculateDelta() {
  auto now = std::chrono::high_resolution_clock::now();
  delta_ = now - previous_time_;
  previous_time_ = std::chrono::high_resolution_clock::now();
}
bool Renderer::checkLoopCallbackAndAdd(std::string name) {
  std::lock_guard<std::mutex> lock(loop_callback_names_mutex_);
  if (loop_callback_names_.find(name) != loop_callback_names_.end()) {
    (*output_stream_) << "Already have the same name function : " << name
                      << " already exsit, please change name" << std::endl;
    return false;
  }
  loop_callback_names_.emplace(name);
  return true;
}
void Renderer::addLoopCallbackFromQueue() {
  {
    std::lock_guard<std::mutex> lock(loop_queue_mutex_);
    while (!loop_callback_adding_queue_.empty()) {
      auto &callback = loop_callback_adding_queue_.front();
      loop_callbacks.emplace(std::get<0>(callback), std::get<1>(callback));
      loop_callback_adding_queue_.pop_front();
    }
  }
}
void Renderer::removeLoopCallbackFromNames() {
  {
    std::lock_guard<std::mutex> lock(loop_remove_mutex_);
    for (auto &name : loop_remove_names_) {
      loop_callbacks.erase(name);
    }
    loop_remove_names_.clear();
  }
}
void Renderer::loopCalls() {
  for (auto &callback : loop_callbacks) {
    callback.second(*this, delta_, swap_chain_index_, frame_);
  }
}
Renderer::Renderer()
    : output_stream_(&std::cout), swap_chain_count_(3), frame_(0),
      delta_(dur(0)), swap_chain_index_(0) {
  D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModelsID, nullptr,
                                  nullptr);
  UINT dxgiFactoryFlags = 0;
  heap_sizes_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV] =
      CBV_SRV_UAV_HEAP_SIZE;
  heap_sizes_[DescriptorType::DESCRIPTOR_TYPE_RTV] = RTV_HEAP_SIZE;
  heap_sizes_[DescriptorType::DESCRIPTOR_TYPE_DSV] = DSV_HEAP_SIZE;
  heap_sizes_[DescriptorType::DESCRIPTOR_TYPE_SAMPLER] = SAMPLER_HEAP_SIZE;

#if defined(_DEBUG)
  ComPtr<ID3D12Debug> debub_controller;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debub_controller)))) {
    debub_controller->EnableDebugLayer();

    // Enable additional debug layers.
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
  } else {
    *output_stream_ << "Fail to enable debug layer" << std::endl;
  }
#endif
  // ComPtr<IDXGIFactory4> factory;
  ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory_)));
  ComPtr<IDXGIAdapter1> hardwareAdapter;
  GetHardwareAdapter(factory_.Get(), &hardwareAdapter, false);
  ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1,
                                  IID_PPV_ARGS(&device_)));
  NAME_D3D12_OBJECT_STRING(device_, std::string("CHCEngine_DX12_Device"));
  checkSupportFeatures();
  createCommandQueues();
  createDescriptorHeap();
  createResourcePool();
  createContexts();
}
bool Renderer::initializeDevice() { return true; }
void Renderer::setSwapChain(Window::Window &window,
                            unsigned int swap_chain_count) {
  window_ = &window;
  auto glfw_window = static_cast<GLFWwindow *>(window.getWindow());
  swap_chain_count_ = swap_chain_count;
  // Describe and create the swap chain.
  auto size = window.getFrameSize();
  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
  swap_chain_desc.BufferCount = swap_chain_count_;
  swap_chain_desc.Width = size.X;
  swap_chain_desc.Height = size.Y;
  swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swap_chain_desc.SampleDesc.Count = 1;

  ComPtr<IDXGISwapChain1> swap_chain;
  ThrowIfFailed(factory_->CreateSwapChainForHwnd(
      context_queues_[CommandType::COMMAND_TYPE_GRAPHICS]
          ->getCommandQueue()
          .Get(), // Swap chain needs the queue so that it can force
                  // a flush on it.
      glfwGetWin32Window(glfw_window), &swap_chain_desc, nullptr, nullptr,
      &swap_chain));
  ThrowIfFailed(factory_->MakeWindowAssociation(glfwGetWin32Window(glfw_window),
                                                DXGI_MWA_NO_ALT_ENTER));
  ThrowIfFailed(swap_chain.As(&swap_chain_));
  swap_chain_buffers_.resize(swap_chain_count_);
  swap_chain_present_fence_.resize(swap_chain_count_);
  for (unsigned int i = 0; i < swap_chain_count_; ++i) {
    ComPtr<GPUResource> buffer;
    ThrowIfFailed(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&buffer)));
    auto swap_chain_range =
        static_heaps_[DescriptorType::DESCRIPTOR_TYPE_RTV]->allocateRange(1);
    device_->CreateRenderTargetView(buffer.Get(), nullptr,
                                    swap_chain_range->getNextAvailableHandle());
    std::string n = "swap_chain_" + std::to_string(i);
    struct tempSwapChain : public Resource::SwapChainBuffer {
      tempSwapChain(ComPtr<GPUResource> buf,
                    std::shared_ptr<DescriptorRange> range, std::string name,
                    int x, int y)
          : SwapChainBuffer(buf, range, name, x, y) {}
    };
    swap_chain_buffers_[i] = std::make_shared<tempSwapChain>(
        buffer, swap_chain_range, n, size.X, size.Y);
    swap_chain_present_fence_[i] = fence_pool_->getContextFence();
    swap_chain_buffers_[i]->getGPUResource();
  }
  loop_thread_ = std::thread(&Renderer::loop, this);
}
void Renderer::presentSwapChain() {
  ThrowIfFailed(swap_chain_->Present(0, 0));
  context_queues_[CommandType::COMMAND_TYPE_GRAPHICS]->insertFenceSignal(
      swap_chain_present_fence_[swap_chain_index_]);
  // swap_chain_present_fence_[swap_chain_index_]
  /*std::cout << swap_chain_->GetCurrentBackBufferIndex() << std::endl
            << std::flush;*/
}
void Renderer::waitUntilWindowClose() {
  if (loop_thread_.joinable())
    loop_thread_.join();
}
bool Renderer::addLoopCallback(std::string name, RenderLoopCallback callback) {
  if (checkLoopCallbackAndAdd(name)) {
    std::lock_guard<std::mutex> lock(loop_queue_mutex_);
    loop_callback_adding_queue_.emplace_back(name, callback);
    return true;
  }
  return false;
}
bool Renderer::checkLoopCallbackNameExist(std::string name) {
  std::lock_guard<std::mutex> lock(loop_callback_names_mutex_);
  return loop_callback_names_.find(name) != loop_callback_names_.end();
}
void Renderer::removeLoopCallback(std::string name) {
  bool find = false;
  {
    std::lock_guard<std::mutex> lock(loop_callback_names_mutex_);
    if (loop_callback_names_.find(name) != loop_callback_names_.end()) {
      find = true;
      loop_callback_names_.erase(name);
    }
  }
  if (find) {
    std::lock_guard<std::mutex> lock(loop_remove_mutex_);
    ;
    loop_remove_names_.emplace_back(name);
  }
}
std::shared_ptr<GraphicsContext>
Renderer::getGraphicsContext(std::function<void(GraphicsContext *)> callback,
                             bool async) {
  return graphics_pool_->getContext(callback, async);
}
std::shared_ptr<ContextFence> Renderer::submitContexts(
    std::shared_ptr<ContextFence> fence,
    std::vector<std::shared_ptr<GraphicsContext>> &&contexts) {
  if (contexts.empty()) {
    throw std::exception("Submit empty context to queue!");
  }
  if (!fence)
    fence = fence_pool_->getContextFence();
  auto type = contexts[0]->getType();
  context_queues_[type]->SubmitContextCommand(contexts, fence);
  return fence;
}
std::shared_ptr<ContextFence> Renderer::getContextFence() {
  return fence_pool_->getContextFence();
}
std::shared_ptr<Pipeline::BindLayout>
Renderer::getBindLayout(const std::vector<Pipeline::BindSlot> &bind_slots) {

  auto sizes = calRootSignatureSize(bind_slots);
  bool relax = true;
  if (sizes.first > root_sigature_size_limits)
    relax = false;
  // pack size still over the 64 , throw
  if (sizes.second > root_sigature_size_limits) {
    throw std::exception(
        "Root signature size is too large, please reorganize the bind slots");
  }
  std::vector<D3D12_ROOT_PARAMETER1> RootParameters;
  std::vector<std::vector<D3D12_DESCRIPTOR_RANGE1>> ranges;
  generateRootParameters(bind_slots, RootParameters, ranges, relax);

  D3D12_VERSIONED_ROOT_SIGNATURE_DESC version_sig_desc;
  version_sig_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
  version_sig_desc.Desc_1_1.Flags =
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
  version_sig_desc.Desc_1_1.pStaticSamplers = nullptr;
  version_sig_desc.Desc_1_1.NumStaticSamplers = 0;
  version_sig_desc.Desc_1_1.NumParameters =
      static_cast<unsigned int>(RootParameters.size());
  version_sig_desc.Desc_1_1.pParameters = RootParameters.data();

  ComPtr<ID3DBlob> signature;
  ComPtr<ID3DBlob> error;
  ComPtr<BindSignature> bind_signature;
  ThrowIfFailed(D3DX12SerializeVersionedRootSignature(
      &version_sig_desc, root_signature_feature.HighestVersion, &signature,
      &error));

  ThrowIfFailed(device_->CreateRootSignature(0, signature->GetBufferPointer(),
                                             signature->GetBufferSize(),
                                             IID_PPV_ARGS(&bind_signature)));
  return std::make_shared<Pipeline::BindLayout>(bind_signature, bind_slots);
}
std::shared_ptr<Pipeline::Pipeline> Renderer::getGraphicsPipeline(
    const ShaderSet &shader_set,
    const std::vector<Resource::Attributes> &attributes,
    std::shared_ptr<BindLayout> bind_layout,
    const RenderTargetSetup &render_setup, const DepthStencilSetup &depth,
    const Rasterizer &rasterizer, PrimitiveTopologyType primitive,
    const Pipeline::SampleSetup &sample) {
  // do the check here..
  D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
  std::vector<D3D12_INPUT_ELEMENT_DESC> inputs;
  std::unordered_map<ShaderType, ComPtr<Blob>> byte_codes;
  for (const auto &shader : shader_set.shader_set_) {
    byte_codes[shader.first] = shader.second.byte_code_;
  }
  fillShaderByteCodes(desc, byte_codes);
  fillInputLayout(inputs, shader_set, attributes);
  desc.InputLayout.NumElements = static_cast<unsigned int>(inputs.size());
  desc.InputLayout.pInputElementDescs = inputs.data();
  desc.pRootSignature = bind_layout->bind_signature_.Get();
  fillBlendState(desc.BlendState, render_setup);
  auto render_formats = render_setup.getRenderFormats();
  for (unsigned int i = 0; i < render_setup.getSize(); ++i) {
    desc.RTVFormats[i] = convertToDXGIFormat(render_formats[i]);
  }
  desc.NumRenderTargets = static_cast<unsigned int>(render_setup.getSize());
  fillDepthStencilState(desc.DepthStencilState, depth);
  desc.DSVFormat = convertToDXGIFormat(depth.getFormat());
  fillRastersizer(desc.RasterizerState, rasterizer);
  desc.PrimitiveTopologyType = convertToD3D12PrimitiveTopologyType(primitive);
  desc.SampleDesc.Count = sample.count_;
  desc.SampleDesc.Quality = sample.quality_;
  desc.SampleMask = UINT_MAX;
  desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
  ComPtr<PipelineState> pipeline;
    ThrowIfFailed(
        device_->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline)));
  return std::make_shared<Pipeline::Pipeline>(
      pipeline, Pipeline::PipelineType::PIPELINE_TYPE_GRAPHICS);
}
} // namespace Renderer
} // namespace CHCEngine
