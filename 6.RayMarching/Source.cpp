#include "CHCEngine.h"

#include "RayMarchHLSLCompt.h"
using CHCEngine::Renderer::ShaderType;
using CHCEngine::Renderer::Pipeline::Pipeline;
using CHCEngine::Renderer::Pipeline::Shader;
using CHCEngine::Renderer::Pipeline::ShaderSet;
using CHCEngine::Renderer::Resource::Texture;
using namespace CHCEngine::Renderer;
uint32_t width = 1920;
uint32_t height = 1080;

int main() {
  CHCEngine::Window::Window window;
  window.openWindow("RayMarching", width, height);
  CHCEngine::Renderer::Renderer renderer;
  renderer.setSwapChain(window);
  Shader full_vertex("FullScreen.hlsl", "VSMain",
                     ShaderType::SHADER_TYPE_VERTEX, true);
  Shader full_pix("FullScreen.hlsl", "PSMain", ShaderType::SHADER_TYPE_PIXEL,
                  true);
  ShaderSet full_set(full_vertex, full_pix);
  auto full_bind_layout = renderer.getBindLayout(full_set.getBindFormats(
      CHCEngine::Renderer::BindType::BIND_TYPE_SIT_ALL));
  auto full_screen_pipeline =
      renderer.getGraphicsPipeline(full_set, {}, full_bind_layout);

  auto sample = renderer.getSampler({});

  enum class UsageIndex : unsigned {
    USAGE_INDEX_SRV,
    USAGE_INDEX_UAV,
    USAGE_INDEX
  };
  std::vector<TextureUsage> usages(
      static_cast<std::underlying_type_t<UsageIndex>>(UsageIndex::USAGE_INDEX));
  usages[static_cast<std::underlying_type_t<UsageIndex>>(
             UsageIndex::USAGE_INDEX_SRV)]
      .data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT;
  usages[static_cast<std::underlying_type_t<UsageIndex>>(
             UsageIndex::USAGE_INDEX_SRV)]
      .usage_ = ResourceUsage::RESOURCE_USAGE_SRV;
  usages[static_cast<std::underlying_type_t<UsageIndex>>(
             UsageIndex::USAGE_INDEX_UAV)]
      .data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT;
  usages[static_cast<std::underlying_type_t<UsageIndex>>(
             UsageIndex::USAGE_INDEX_UAV)]
      .usage_ = ResourceUsage::RESOURCE_USAGE_UAV;

  auto frame_buffer = renderer.getTexture(TextureType::TEXTURE_TYPE_2D,
                                          RawFormat::RAW_FORMAT_R32G32B32A32,
                                          width, height, 1, 1, usages);

  Shader trace_shader("RayMarchSphere.hlsl", "CSMain",
                      ShaderType::SHADER_TYPE_COMPUTE, true);
  auto trace_bind_layout = renderer.getBindLayout(
      trace_shader.getBindFormats(BindType::BIND_TYPE_SIT_ALL));
  auto trace_pipeline =
      renderer.getComputePipeline(trace_shader, trace_bind_layout);

  Shader tetrahedron_shader("RayMarchTetrahedron.hlsl", "CSMain",
                            ShaderType::SHADER_TYPE_COMPUTE, true, {"Time"});
  auto tetrahedron_bind_layout = renderer.getBindLayout(
      tetrahedron_shader.getBindFormats(BindType::BIND_TYPE_SIT_ALL));
  auto tetrahedron_pipeline =
      renderer.getComputePipeline(tetrahedron_shader, tetrahedron_bind_layout);

  Shader julia_set_shader("RayMarchJuliaSet.hlsl", "CSMain",
                          ShaderType::SHADER_TYPE_COMPUTE, true, {"Time"});
  auto julia_set_bind_layout = renderer.getBindLayout(
      julia_set_shader.getBindFormats(BindType::BIND_TYPE_SIT_ALL));
  auto julia_set_pipeline =
      renderer.getComputePipeline(julia_set_shader, julia_set_bind_layout);

  Shader julia_4d_shader("RayMarchJulia4D.hlsl", "CSMain",
                         ShaderType::SHADER_TYPE_COMPUTE, true, {"Time"});
  auto julia_4d_bind_layout = renderer.getBindLayout(
      julia_4d_shader.getBindFormats(BindType::BIND_TYPE_SIT_ALL));
  auto julia_4d_pipeline =
      renderer.getComputePipeline(julia_4d_shader, julia_4d_bind_layout);

  std::vector<std::shared_ptr<CHCEngine::Renderer::ContextFence>>
      graphics_fences(3u);
  std::vector<std::shared_ptr<CHCEngine::Renderer::GraphicsContext>>
      graphics_contexts(3u);
  for (uint32_t i = 0; i < 3; ++i) {
    graphics_contexts[i] = renderer.getGraphicsContext();
    graphics_fences[i] = renderer.getContextFence();
  }
  CHCEngine::Renderer::Pipeline::Viewport viewport(0, 0, width, height);
  CHCEngine::Renderer::Pipeline::Scissor scissor(width, height);
  auto start = std::chrono::system_clock::now();
  renderer.addLoopCallback(
      "RenderLoop",
      [&](CHCEngine::Renderer::Renderer &renderer,
          std::chrono::duration<long long, std::nano> const &delta,
          unsigned int swap_chain_index, unsigned long long const &frame) {
        uint32_t wait_index = (swap_chain_index + 2) % 3u;
        uint32_t current = swap_chain_index;
        // graphics_contexts[current]->setPipeline(trace_pipeline);
        // graphics_contexts[current]->setComputeBindLayout(trace_bind_layout);
        /*graphics_contexts[current]->setPipeline(tetrahedron_pipeline);
        graphics_contexts[current]->setComputeBindLayout(
            tetrahedron_bind_layout);*/

        /*graphics_contexts[current]->setPipeline(julia_set_pipeline);
        graphics_contexts[current]->setComputeBindLayout(julia_set_bind_layout);*/

        graphics_contexts[current]->setPipeline(julia_4d_pipeline);
        graphics_contexts[current]->setComputeBindLayout(julia_4d_bind_layout);

        auto now = std::chrono::system_clock::now();
        std::chrono::duration<float> dur = now - start;
        float second = dur.count();
        graphics_contexts[current]->bindComputeConstants(&second, 1, "Time");

        graphics_contexts[current]->bindComputeResource(
            frame_buffer, "frame_buffer",
            static_cast<std::underlying_type_t<UsageIndex>>(
                UsageIndex::USAGE_INDEX_UAV));

        auto [x, y, z] = trace_pipeline->getComputeThreadSize();
        graphics_contexts[current]->dispatch(width / x + 1, height / y + 1,
                                             1 / z);

        graphics_contexts[current]->setRenderTarget(
            renderer.getSwapChainBuffer(current));
        graphics_contexts[current]->setPipeline(full_screen_pipeline);

        graphics_contexts[current]->setGraphicsBindLayout(full_bind_layout);
        graphics_contexts[current]->bindGraphicsResource(
            frame_buffer, "frame_buffer",
            static_cast<std::underlying_type_t<UsageIndex>>(
                UsageIndex::USAGE_INDEX_SRV));
        graphics_contexts[current]->bindGraphicsSampler(sample, "sample");
        graphics_contexts[current]->setViewport(viewport);
        graphics_contexts[current]->setScissor(scissor);
        graphics_contexts[current]->setPrimitiveTopology(
            CHCEngine::Renderer::PrimitiveTopology::
                PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        graphics_contexts[current]->drawInstanced(3);
        graphics_contexts[current]->setSwapChainToPresetState(
            renderer.getSwapChainBuffer(current));
        renderer.waitFenceSubmitContexts(graphics_fences[wait_index],
                                         graphics_fences[current],
                                         {graphics_contexts[current]});
        renderer.presentSwapChain();
      });

  window.waitUntilClose();
  renderer.waitUntilWindowClose();
}