#include <chrono>
#include <iostream>
#include <sstream>

#include "CHCEngine.h"
#include "Renderer/Pipeline/Shader.h"

#include <functional>

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
  window.openWindow("Triangle", 800, 600);
  renderer.initializeDevice();
  renderer.setSwapChain(window);

  std::string code = "struct PSInput\
  {\
    float4 position : SV_POSITION;\
    float3 color : COLOR;\
  };\
  cbuffer SceneConstBuffer : register(b0) {\
    float4 scenecolor;\
  };\
  StructuredBuffer<float4> Color: register(t0);\
  PSInput VSMain(uint id: SV_VertexID, float2 position : POSITION, float3 color : COLOR) {\
    PSInput result;\
    result.position = float4(position, 1.0, 1.0);\
    result.color = (Color[id].xyz+scenecolor.xyz+color)/3.0;\
    return result;\
  }\
  half4 PSMain(PSInput input) : SV_TARGET {\
    return float4(input.color,0);\
  }";

  std::string compute = "struct Particle\
  {\
    float3 Pos;\
    float3 Vel;\
    float3 Color;\
    float life;\
  };\
  cbuffer SceneConstBuffer : register(b0) {\
    float delta;\
    uint ParNum;\
  };\
  RWStructuredBuffer<Particle> NextState : register(u0);\
  StructuredBuffer<Particle> PrevState : register(t0);\
  float rand_1_05(float2 uv) {\
    float2 noise =\
        (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));\
    return abs(noise.x + noise.y) * 0.5;\
  }\
  [numthreads(1024, 1, 1)] void CSMain(uint3 id\
                                       : SV_DispatchThreadID, uint3 tid\
                                       : SV_GroupThreadID) {\
    if (id.x < ParNum) {\
      if (PrevState[id.x].life > 10.0f) {\
        float ran = rand_1_05(float2(id.x, tid.x));\
        float ran2 = rand_1_05(float2(id.x, tid.x + 1));\
        float ran3 = rand_1_05(float2(id.x, tid.x - 1));\
        NextState[id.x].Pos = float3(ran3 - 0.5, 0, -1 * (ran - 0.5));\
        NextState[id.x].Vel = float3(ran - 0.5, (ran2 - 0.5), ran3 - 0.5);\
        NextState[id.x].life = 0.0;\
        NextState[id.x].Color = float3(0.3, NextState[id.x].life / 10,\
                                       1 - NextState[id.x].life / 10);\
      } else {\
        NextState[id.x].Pos = PrevState[id.x].Pos + PrevState[id.x].Vel * \delta;\
        float3 flat = NextState[id.x].Pos.xyz;\
        flat.y = 0;\
        float3 force = normalize(flat) / length(NextState[id.x].Pos);\
        NextState[id.x].Vel = PrevState[id.x].Vel - (force * delta) * 0.5;\
        NextState[id.x].life = PrevState[id.x].life + delta;\
        NextState[id.x].Color = float3(0.1, abs(NextState[id.x].Pos.y / 5),\
                                       1 - NextState[id.x].life / 10);\
      }\
    }\
  }";

  std::string entry = "PSMain";

  Shader sh(code, "PSMain", ShaderType::SHADER_TYPE_PIXEL);
  Shader sh2(code, "VSMain", ShaderType::SHADER_TYPE_VERTEX);
  Shader computeshader(compute, "CSMain", ShaderType::SHADER_TYPE_COMPUTE);
  ShaderSet shset(sh, sh2);

  const auto input = sh.getInputTable();

  auto compute_bind = computeshader.getBindFormats(BindType::BIND_TYPE_SIT_ALL);

  auto compute_bind_layout =
      renderer.getBindLayout({Pipeline::BindSlot({compute_bind})});

  // auto input = sh.getOutputTable();
  auto exsample = shset.getBindFormatsExclude(BindType::BIND_TYPE_SIT_SAMPLER);

  auto sample = shset.getBindFormats(BindType::BIND_TYPE_SIT_SAMPLER);

  // Pipeline::BindSlot exsampleslot("resource slot", exsample);
  // Pipeline::BindSlot sampleslot(sample);
  // Pipeline::BindSlot dummyslot;
  // std::vector<Pipeline::BindFormat> v = {shset.getBindFormat("g_texture")};

  std::vector<Pipeline::BindSlot> slots = {
      Pipeline::BindSlot({shset.getBindFormat("Color")}),
      Pipeline::BindSlot({shset.getBindFormat("SceneConstBuffer")})};
  auto bind_layout = renderer.getBindLayout(exsample);
  bind_layout->setName("simple layout");

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> buffer =
      renderer.getBuffer(
          3, 20,
          {{.usage_ = ResourceUsage::RESOURCE_USAGE_SRV,
            .is_raw_buffer_ = true}},
          ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC,
          {{"POSITION", DataFormat::DATA_FORMAT_R32G32_FLOAT},
           {"COLOR", DataFormat::DATA_FORMAT_R32G32B32_FLOAT}});
  buffer->setName("custom vertex buffer");

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> frame_count_buffer =
      renderer.getVertexBuffer(
          1, {{"POSITION", DataFormat::DATA_FORMAT_R32_UINT}},
          ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC);
  frame_count_buffer->setName("frame count buffer");

  float tridata[] = {0.0f, 0.25f, 1.0f,   0.0f,   0.0f, 0.25f, -0.25f, 0.0f,
                     1.0f, 0.0f,  -0.25f, -0.25f, 0.0f, 0.0f,  1.0f};

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

  Color texture_data[10] = {{1.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 0.0, 1.0},
                            {0.0, 0.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0},
                            {0.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0, 1.0},
                            {1.0, 1.0, 0.0, 1.0}, {1.0, 0.0, 1.0, 1.0},
                            {0.0, 0.0, 0.0, 1.0}, {1.0, 1.0, 1.0, 1.0}};
  auto simple_texture = renderer.getTexture(
      TextureType::TEXTURE_TYPE_3D, RawFormat::RAW_FORMAT_R32G32B32A32, 2, 2, 2,
      2,
      {{
          .usage_ = ResourceUsage::RESOURCE_USAGE_SRV,
          .data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT,
      }},
      {}, {}, ResourceState::RESOURCE_STATE_COMMON,
      Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC);

  auto copycontext = renderer.getGraphicsContext();
  copycontext->setStaticUsageHeap();
  copycontext->updateBuffer(color_buffer, colors,
                            color_buffer->getBufferInformation().size_);

  Color color{1.0f, 0.2f, 0.6f, 0.0f};
  copycontext->updateBuffer(constant_buffer, &color, sizeof(color));
  //copycontext->updateTexture(simple_texture, texture_data);
  copycontext->resourceTransition(
      constant_buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
      ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  copycontext->resourceTransition(
      color_buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
      ResourceState::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
  renderer.submitContexts(nullptr, copycontext)->waitComplete();

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> index_buffer =
      renderer.getIndexBuffer(6);

  auto pipeline = renderer.getGraphicsPipeline(
      shset, {buffer->getBufferInformation().vetex_attributes_}, bind_layout);
  pipeline->setName("simple pipeline");

   std::shared_ptr<ContextFence> g1;
  ;
  std::shared_ptr<ContextFence> c1;

  auto graphics = renderer.getGraphicsContext();
  auto graphics2 = renderer.getGraphicsContext();
  auto copycxt = renderer.getCopyContext();

  Pipeline::Viewport view_port(window.getFrameSize().X,
                               window.getFrameSize().Y);
  Pipeline::Scissor scissor(window.getFrameSize().X, window.getFrameSize().Y);

  int sign[3] = {1, 1, 1};

  renderer.addLoopCallback("Render", [&](Renderer &renderer, auto duration,
                                         auto swap_chain_index, auto frame) {
    graphics->recordCommands<CHCEngine::Renderer::Context::GraphicsContext>(
        [&](CHCEngine::Renderer::Context::GraphicsContext *graph) {
          graph->resourceTransition(
              constant_buffer,
              ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
              ResourceState::RESOURCE_STATE_COPY_DEST);
          graph->resourceTransition(
              buffer, ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
              ResourceState::RESOURCE_STATE_COPY_DEST);
          graph->resourceTransition(cube_textures,
                                    ResourceState::RESOURCE_STATE_COPY_DEST,
                                    ResourceState::RESOURCE_STATE_DEPTH_WRITE);
          graph->resourceTransition(
              renderer.getSwapChainBuffer(swap_chain_index),
              ResourceState::RESOURCE_STATE_PRESENT,
              ResourceState::RESOURCE_STATE_RENDER_TARGET, true);
          tridata[1] = ((float)(rand() % 1000)) / 1000.0f;
          tridata[7] += sign[0] * 0.001f;
          if (tridata[7] >= 1.0)
            sign[0] = -1;
          if (tridata[7] <= 0.0)
            sign[0] = 1;
          tridata[8] += sign[1] * 0.001f;
          if (tridata[8] >= 1.0)
            sign[1] = -1;
          if (tridata[8] <= 0.0)
            sign[1] = 1;
          tridata[9] += sign[2] * 0.001f;
          if (tridata[9] >= 1.0)
            sign[2] = -1;
          if (tridata[9] <= 0.0)
            sign[2] = 1;
          graph->updateBuffer(buffer, tridata,
                              buffer->getBufferInformation().size_);
          graph->clearRenderTarget(
              renderer.getSwapChainBuffer(swap_chain_index),
              {0.1f, 0.6f, 0.7f, 0.0f});
          graph->resourceTransition(cube_textures,
                                    ResourceState::RESOURCE_STATE_DEPTH_WRITE,
                                    ResourceState::RESOURCE_STATE_COPY_DEST);
          graph->resourceTransition(
              constant_buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
              ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
          graph->resourceTransition(
              buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
              ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, true);
          graph->setPipeline(pipeline);
          graph->setGraphicsBindLayout(bind_layout);
          graph->bindGraphicsResource(color_buffer, "Color");
          graph->bindGraphicsResource(constant_buffer, "SceneConstBuffer");
          graph->setVertexBuffers(buffer);
          graph->setViewport(view_port);
          graph->setScissor(scissor);
          graph->setPrimitiveTopology(
              PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST);
          graph->setRenderTarget(renderer.getSwapChainBuffer(swap_chain_index));
          graph->drawInstanced(3);
          graph->resourceTransition(
              renderer.getSwapChainBuffer(swap_chain_index),
              ResourceState::RESOURCE_STATE_RENDER_TARGET,
              ResourceState::RESOURCE_STATE_PRESENT, true);
        },
        false);

    copycxt->updateTexture(simple_texture, texture_data);
    renderer.submitContexts(nullptr, copycxt);
    renderer.submitContexts(nullptr, graphics);
    renderer.presentSwapChain();
  });
  renderer.waitUntilWindowClose();
  window.waitUntilClose();
  return 0;
}