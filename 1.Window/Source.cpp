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
using CHCEngine::Window::Action;
using CHCEngine::Window::Condition;
using CHCEngine::Window::Key;
using CHCEngine::Window::MouseButton;
using CHCEngine::Window::Vector;
using CHCEngine::Window::Window;
using namespace CHCEngine::Renderer;

int t = 0;
Renderer renderer;
int main() {
  Window window;
  window.openWindow("Window", 800, 600);
  window.setFrameTimeLowerBound(30000000);
  std::cout << "simple window test" << std::endl;
  window.addKeyCallBack(
      "test call back",
      [&](Key key, Action action) {
        std::cout << t++ << std::endl;
        window.removeInputCallback("test call back");
      },
      Key::KEY_W, Action::ACTION_PRESS,
      {{{Key::KEY_S, Action::ACTION_REPEAT}},
       {{MouseButton::MOUSE_BUTTON_LEFT, Action::ACTION_REPEAT}}});
  window.addKeyCallBack(
      "test call back2",
      [&](Key key, Action action) { std::cout << t-- << std::endl; },
      Key::KEY_W, Action::ACTION_PRESS,
      {{{Key::KEY_S, Action::ACTION_REPEAT}},
       {{MouseButton::MOUSE_BUTTON_LEFT, Action::ACTION_REPEAT}}});
  Condition c = {{{Key::KEY_LEFT_SHIFT, Action::ACTION_REPEAT}},
                 {{MouseButton::MOUSE_BUTTON_LEFT, Action::ACTION_REPEAT}}};
  window.addMouseMoveCallback(
      "mouse_move",
      [](const Vector &position, const Vector &offset) {
        std::cout << "Position : " << position.X << "   " << position.Y
                  << std::endl;
      },
      c);
  window.addMouseButtonCallback(
      "mouse_button",
      [](MouseButton mouse_button, Action action, const Vector &position) {
        std::cout << "Mouse right click" << std::endl;
      },
      MouseButton::MOUSE_BUTTON_RIGHT, Action::ACTION_PRESS, c);
  window.addMouseButtonCallback(
      "mouse_button",
      [](MouseButton mouse_button, Action action, const Vector &position) {
        std::cout << "Mouse right click  2" << std::endl;
      },
      MouseButton::MOUSE_BUTTON_RIGHT, Action::ACTION_PRESS, c);
  window.addFramebufferSizeCallback("window_sizse", [](int width, int height) {
    std::cout << " new size : " << width << "  " << height << std::endl;
  });

  /*window.addLoopCallback("loop_start", [&](auto duration, uint64_t frame_) {
    std::cout << " start "<< std::endl;
    if (frame_ > 30000) window.removeLoopCallback("loop_start");
  },CHCEngine::Window::LoopCallbackType::LOOP_CALLBACK_TYPE_START);*/
  /*window.addKeyCallBack("test call back2", [&](Key key, Action action) {
          std::cout << t-- << std::endl;
          }, Key::KEY_W, Action::ACTION_PRESS);*/
  renderer.initializeDevice();
  // std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  renderer.setSwapChain(window);
  // std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  // window.removeMouseButtonCallback("mouse_button_2");
  // window.removeInputCallback("mouse_move");
  /*renderer.presentSwapChain();
  renderer.presentSwapChain();
  renderer.presentSwapChain();*/
  /*window.addLoopCallback("loop_main", [&](auto duration, uint64_t frame_) {
    std::cout << duration.count() / 1000000000.0 << "   " << frame_
              << std::endl;
  });*/
  // auto pre_context = renderer.getGraphicsContext;

  std::string code = "struct PSInput\
  {\
    float4 position : SV_POSITION;\
    float3 color : COLOR;\
  };\
  PSInput VSMain(float2 position : POSITION, float3 color : COLOR) {\
    PSInput result;\
    result.position = float4(position, 1.0, 1.0);\
    result.color = color;\
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
      renderer.getBindLayout({{"compute point", compute_bind}});

  // auto input = sh.getOutputTable();
  auto exsample = shset.getBindFormatsExclude(BindType::BIND_TYPE_SIT_SAMPLER);

  auto sample = shset.getBindFormats(BindType::BIND_TYPE_SIT_SAMPLER);

  // Pipeline::BindSlot exsampleslot("resource slot", exsample);
  // Pipeline::BindSlot sampleslot(sample);
  // Pipeline::BindSlot dummyslot;
  // std::vector<Pipeline::BindFormat> v = {shset.getBindFormat("g_texture")};
  auto bind_layout = renderer.getBindLayout({});
  bind_layout->setName("Empty layout");

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> buffer =
      renderer.getVertexBuffer(
          3,
          {{"POSITION", DataFormat::DATA_FORMAT_R32G32_FLOAT},
           {"COLOR", DataFormat::DATA_FORMAT_R32G32B32_FLOAT}},
          ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC);
  buffer->setName("vertex buffer");

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> frame_count_buffer =
      renderer.getVertexBuffer(
          1, {{"POSITION", DataFormat::DATA_FORMAT_R32_UINT}},
          ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC);
  frame_count_buffer->setName("frame count buffer");

  float tridata[] = {0.0f, 0.25f, 1.0f,   0.0f,   0.0f, 0.25f, -0.25f, 0.0f,
                     1.0f, 0.0f,  -0.25f, -0.25f, 0.0f, 0.0f,  1.0f};

  /*auto copycontext = renderer.getGraphicsContext();

  copycontext->updateBuffer(buffer, tridata,
                            buffer->getBufferInformation().size_);
  copycontext->resourceTransition(
      buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
      ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, true);
  renderer.submitContexts(nullptr, copycontext)->waitComplete();*/

  std::shared_ptr<CHCEngine::Renderer::Resource::Buffer> index_buffer =
      renderer.getIndexBuffer(6);

  auto pipeline = renderer.getGraphicsPipeline(
      shset, {buffer->getBufferInformation().vetex_attributes_}, bind_layout);
  pipeline->setName("simple pipeline");

  auto graphics = renderer.getGraphicsContext();

  Pipeline::Viewport view_port(window.getFrameSize().X,
                               window.getFrameSize().Y);
  Pipeline::Scissor scissor(window.getFrameSize().X, window.getFrameSize().Y);

  renderer.addLoopCallback("Render", [&](Renderer &renderer, auto duration,
                                         auto swap_chain_index, auto frame) {
    graphics->recordCommands<CHCEngine::Renderer::Context::GraphicsContext>(
        [&](CHCEngine::Renderer::Context::GraphicsContext *graph) {
          graph->resourceTransition(
              buffer, ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
              ResourceState::RESOURCE_STATE_COPY_DEST);
          graph->resourceTransition(
              renderer.getSwapChainBuffer(swap_chain_index),
              ResourceState::RESOURCE_STATE_PRESENT,
              ResourceState::RESOURCE_STATE_RENDER_TARGET, true);
         // tridata[1] = ((float)(rand() % 1000)) / 1000.0f;
          tridata[7] = ((float)(rand() % 1000)) / 1000.0f;
          tridata[8] = ((float)(rand() % 1000)) / 1000.0f;
          tridata[9] = ((float)(rand() % 1000)) / 1000.0f;
          graph->updateBuffer(buffer, tridata,
                              buffer->getBufferInformation().size_);
          graph->clearRenderTarget(
              renderer.getSwapChainBuffer(swap_chain_index),
              {0.1f, 0.6f, 0.7f, 0.0f});
          graph->resourceTransition(
              buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
              ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, true);
        },
        false);
    graphics->setPipeline(pipeline);
    graphics->setGraphicsBindLayout(bind_layout);
    graphics->setVertexBuffers(buffer);
    graphics->setViewport(view_port);
    graphics->setScissor(scissor);
    graphics->setPrimitiveTopology(
        PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    graphics->setRenderTarget(renderer.getSwapChainBuffer(swap_chain_index));
    graphics->drawInstanced(3);
    graphics->resourceTransition(renderer.getSwapChainBuffer(swap_chain_index),
                                 ResourceState::RESOURCE_STATE_RENDER_TARGET,
                                 ResourceState::RESOURCE_STATE_PRESENT, true);
    renderer.submitContexts(nullptr, graphics);
    renderer.presentSwapChain();
  });
  renderer.waitUntilWindowClose();
  window.waitUntilClose();
  return 0;
}