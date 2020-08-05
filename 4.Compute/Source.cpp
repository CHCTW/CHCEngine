#include <chrono>
#include <iostream>
#include <sstream>

#include "CHCEngine.h"
#include "Renderer/Pipeline/Shader.h"

#include <functional>
#define THREADSIZE 8

using CHCEngine::Renderer::BindType;
using CHCEngine::Renderer::DataFormat;
using CHCEngine::Renderer::Renderer;
using CHCEngine::Renderer::ResourceState;
using CHCEngine::Renderer::ResourceTransitionFlag;
using CHCEngine::Renderer::ShaderType;
using CHCEngine::Renderer::Pipeline::Shader;
using CHCEngine::Renderer::Pipeline::ShaderSet;
using CHCEngine::Window::Window;
using namespace CHCEngine::Renderer;

int t = 0;
Renderer renderer;
int main() {
  Window window;
  window.openWindow("Compute", 800, 600);
  renderer.initializeDevice();
  renderer.setSwapChain(window);

  std::string code = "struct PSInput\
  {\
    float4 position : SV_POSITION;\
    float3 color: COLOR;\
    float2 uv : UV;\
  };\
  cbuffer SceneConstBuffer : register(b0) {\
    float4 scenecolor;\
  };\
  StructuredBuffer<float4> Color: register(t0);\
  Texture2D simple_texture : register(t1);\
  SamplerState simple_sampler : register(s1);\
  PSInput VSMain(uint id: SV_VertexID, float2 position : POSITION, float2 uv : UV) {\
    PSInput result;\
    result.position = float4(position, 1.0, 1.0);\
    result.color = (Color[id].xyz+scenecolor.xyz)/2.0;\
    result.uv = uv;\
    return result;\
  }\
  half4 PSMain(PSInput input) : SV_TARGET {\
    return float4(input.color,0)+simple_texture.Sample(simple_sampler,input.uv);\
  }";

  std::string compute = "compute.hlsl";

  std::string entry = "PSMain";

  Shader sh(code, "PSMain", ShaderType::SHADER_TYPE_PIXEL);
  Shader sh2(code, "VSMain", ShaderType::SHADER_TYPE_VERTEX);
  Shader computeshader(compute, "CSMain", ShaderType::SHADER_TYPE_COMPUTE,
                       true);
  ShaderSet shset(sh, sh2);

  const auto input = sh.getInputTable();

  auto compute_bind = computeshader.getBindFormats(BindType::BIND_TYPE_SIT_ALL);

  auto compute_bind_layout =
      renderer.getBindLayout({Pipeline::BindSlot({compute_bind})});
  auto compute_pipe =
      renderer.getComputePipeline(computeshader, compute_bind_layout);

  auto exsample = shset.getBindFormatsExclude(BindType::BIND_TYPE_SIT_SAMPLER);

  auto sample = shset.getBindFormats(BindType::BIND_TYPE_SIT_SAMPLER);

  // all bind int the same slot will share the same visibility,
  // thus simple_texture need to has state with
  // pixel| non_pixel when binding....
  // this could be better when we have auto transition
  std::vector<Pipeline::BindSlot> slots = {
      Pipeline::BindSlot({shset.getBindFormat("Color"),
                          shset.getBindFormat("SceneConstBuffer"),
                          shset.getBindFormat("simple_texture")}),
      {sample}};
  auto bind_layout = renderer.getBindLayout(exsample);
  auto groups_bind_layout = renderer.getBindLayout(slots);
  bind_layout->setName("simple layout");

  auto res_group = renderer.getResourceGroup(3);

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> buffer =
      renderer.getBuffer(
          3, 16,
          {{.usage_ = ResourceUsage::RESOURCE_USAGE_SRV,
            .is_raw_buffer_ = true}},
          ResourceState::RESOURCE_STATE_COMMON,
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC,
          {{"POSITION", DataFormat::DATA_FORMAT_R32G32_FLOAT},
           {"UV", DataFormat::DATA_FORMAT_R32G32_FLOAT}});
  buffer->setName("custom vertex buffer");

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> frame_count_buffer =
      renderer.getVertexBuffer(
          1, {{"POSITION", DataFormat::DATA_FORMAT_R32_UINT}},
          ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC);
  frame_count_buffer->setName("frame count buffer");

  float tridata[] = {0.0f,  0.25f, 0.0f,   0.0f,   0.25f, -0.25f,
                     1.0f, 0.0f,  -0.25f, -0.25f, 1.0f, 1.0f};

  Color colors[3] = {
      {0.0, 0.0, 0.0, 1.0}, {0.5, 0.5, 0.5, 1.0}, {1.0, 1.0, 1.0, 1.0}};

  auto color_buffer = renderer.getBuffer(
      3, sizeof(Color), {{.usage_ = ResourceUsage::RESOURCE_USAGE_SRV}});

  auto constant_buffer = renderer.getBuffer(
      1, sizeof(Color), {{.usage_ = ResourceUsage::RESOURCE_USAGE_CBV}});
  constant_buffer->setName("constant buffer");

  MipRange mips;
  mips.mips_start_level_ = 2;
  SubTexturesRange textue_range;

  auto texture = renderer.getTexture(
      TextureType::TEXTURE_TYPE_2D, RawFormat::RAW_FORMAT_B8G8R8A8, 256, 256, 1,
      5,
      {{.usage_ = ResourceUsage::RESOURCE_USAGE_SRV,
        .data_format_ = DataFormat::DATA_FORMAT_B8G8R8A8_UNORM,
        .mip_range_ = mips}},
      {{.data_format_ = DataFormat::DATA_FORMAT_B8G8R8A8_UNORM,
        .mip_slice_ = 3}});

  auto cube_textures = renderer.getTexture(
      TextureType::TEXTURE_TYPE_2D, RawFormat::RAW_FORMAT_R24G8, 512, 512, 12,
      5,
      {{
          .usage_ = ResourceUsage::RESOURCE_USAGE_SRV,
          .data_format_ = DataFormat::DATA_FORMAT_R24_UNORM_X8_TYPELESS,
          .data_dimension_ = DataDimension::DATA_DIMENSION_TEXTURECUBEARRAY,
      }},
      empty_render_target_usage,
      {{.data_format_ =
            DepthStencilFormat::DEPTH_STENCIL_FORMAT_D24_UNORM_S8_UINT,
        .mip_slice_ = 4}});
  auto sterio_texture = renderer.getTexture(
      TextureType::TEXTURE_TYPE_3D, RawFormat::RAW_FORMAT_R32G32B32A32, 32, 32,
      32, 5,
      {{
           .usage_ = ResourceUsage::RESOURCE_USAGE_SRV,
           .data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT,
       },
       {
           .usage_ = ResourceUsage::RESOURCE_USAGE_UAV,
           .data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT,
       }},
      {{.data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT}});

  auto simple_texture = renderer.getTexture(
      TextureType::TEXTURE_TYPE_2D, RawFormat::RAW_FORMAT_R32G32B32A32, 800, 800, 1,
      1,
      {{
           .usage_ = ResourceUsage::RESOURCE_USAGE_SRV,
           .data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT,
       },
       {
           .usage_ = ResourceUsage::RESOURCE_USAGE_UAV,
           .data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT,
       }});
  Sampler::SamplerInformation sample_set = {
      .u_mode_ = TextureAddressMode::TEXTURE_ADDRESS_MODE_WRAP,
      .v_mode_ = TextureAddressMode::TEXTURE_ADDRESS_MODE_MIRROR};
  res_group->insertResource(0, color_buffer);
  res_group->insertResource(1, constant_buffer);
  res_group->insertResource(2, simple_texture);

  auto sampler = renderer.getSampler(sample_set);

  auto sampler_group = renderer.getSamplerGroup(1);
  sampler_group->insertSampler(0, sampler);

  auto copycontext = renderer.getGraphicsContext();
  copycontext->setStaticUsageHeap();
  copycontext->updateBuffer(color_buffer, colors,
                            color_buffer->getBufferInformation().size_);

  Color color{1.0f, 0.2f, 0.6f, 0.0f};
  copycontext->updateBuffer(constant_buffer, &color, sizeof(color));
  copycontext->updateBuffer(buffer, &tridata,
                            buffer->getBufferInformation().size_);
  copycontext->resourceTransition(
      simple_texture, ResourceState::RESOURCE_STATE_COPY_DEST,
      ResourceState::RESOURCE_STATE_UNORDERED_ACCESS, true
      );
 /* copycontext->resourceTransition(
      constant_buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
      ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  copycontext->resourceTransition(
      buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
      ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  copycontext->resourceTransition(
      color_buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
      ResourceState::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);*/
  renderer.submitContexts({copycontext})->waitComplete();

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> index_buffer =
      renderer.getIndexBuffer(6);

  auto pipeline = renderer.getGraphicsPipeline(
      shset, {buffer->getBufferInformation().vetex_attributes_},
      groups_bind_layout);
  pipeline->setName("simple pipeline");

  auto graphics = renderer.getGraphicsContext();
  renderer.submitContexts({graphics})->waitComplete();

  Pipeline::Viewport view_port(window.getFrameSize().X,
                               window.getFrameSize().Y);
  Pipeline::Scissor scissor(window.getFrameSize().X, window.getFrameSize().Y);

  auto graphics_fence = renderer.getContextFence();
  auto compute_fence = renderer.getContextFence();
  auto compute_context = renderer.getComputeContext();

  renderer.addLoopCallback("Render", [&](Renderer &renderer, auto duration,
                                         auto swap_chain_index, auto frame) {
    compute_context->setPipeline(compute_pipe);
    compute_context->setComputeBindLayout(compute_bind_layout);
    compute_context->bindComputeResource(simple_texture, "texts", 1);
    compute_context->dispatch(
        (unsigned int)simple_texture->getTextureInformation().width_ / THREADSIZE + 1,
        (unsigned int)simple_texture->getTextureInformation().height_ /
                THREADSIZE +
            1,
        1);


    graphics->recordCommands<CHCEngine::Renderer::Context::GraphicsContext>(
        [&](CHCEngine::Renderer::Context::GraphicsContext *graph) {
          graph->resourceTransition(
              simple_texture, ResourceState::RESOURCE_STATE_UNORDERED_ACCESS,
              ResourceState::RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                  ResourceState::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
          graph->resourceTransition(
              renderer.getSwapChainBuffer(swap_chain_index),
              ResourceState::RESOURCE_STATE_PRESENT,
              ResourceState::RESOURCE_STATE_RENDER_TARGET, true);
          graph->clearRenderTarget(
              renderer.getSwapChainBuffer(swap_chain_index),
              {0.1f, 0.6f, 0.7f, 0.0f});
          graph->setPipeline(pipeline);
          graph->setGraphicsBindLayout(groups_bind_layout);
          graph->bindGraphicsResource(res_group, "Color");
          graph->bindGraphicsSamplers(sampler_group, "simple_sampler");
          graph->setVertexBuffers(buffer);
          graph->setViewport(view_port);
          graph->setScissor(scissor);
          graph->setPrimitiveTopology(
              PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST);
          graph->setRenderTarget(renderer.getSwapChainBuffer(swap_chain_index));
          graph->drawInstanced(3);
          graph->resourceTransition(
              simple_texture,
                  ResourceState::RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
            ResourceState::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        ResourceState::RESOURCE_STATE_UNORDERED_ACCESS);
          graph->resourceTransition(
              renderer.getSwapChainBuffer(swap_chain_index),
              ResourceState::RESOURCE_STATE_RENDER_TARGET,
              ResourceState::RESOURCE_STATE_PRESENT, true);
        },
        false);
    renderer.waitFenceSubmitContexts(graphics_fence, compute_fence,
                                     {compute_context});
    renderer.waitFenceSubmitContexts(compute_fence,graphics_fence, {graphics});
    renderer.presentSwapChain();
  });
  renderer.waitUntilWindowClose();
  window.waitUntilClose();
  return 0;
}