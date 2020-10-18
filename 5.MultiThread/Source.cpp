#include "CHCEngine.h"
#include "QuadHlslCompat.h"
#include <algorithm>
#include <functional>
#include <mutex>
#include <queue>
#include <random>

const static uint32_t width = 800;
const static uint32_t height = 600;
const static uint32_t quad_count = 1000;
const static uint32_t quad_update_thrad_count = 4;
const static uint32_t quad_draw_thrad_count = 16;

using CHCEngine::Renderer::BindType;
using CHCEngine::Renderer::DataFormat;
using CHCEngine::Renderer::Renderer;
using CHCEngine::Renderer::ResourceState;
using CHCEngine::Renderer::ResourceTransitionFlag;
using CHCEngine::Renderer::ShaderType;
using CHCEngine::Renderer::Pipeline::Shader;
using CHCEngine::Renderer::Pipeline::ShaderSet;
using CHCEngine::Window::Vector;
using CHCEngine::Window::Window;
using namespace CHCEngine::Renderer;

struct Velocity {
  float x_ = 0.0;
  float y_ = 0.0;
};
struct Quad {
  Position position_;
  Velocity velocity_;
};
#define PI 3.14159265f
std::default_random_engine generator;
std::uniform_real_distribution<float> angle_distribution(0.0f, PI * 2);
std::uniform_real_distribution<float> magnitue_distribution(0.0001f, 0.005f);
std::uniform_real_distribution<float> z_distribution(0.0f, 0.9999f);

int main() {

  CHCEngine::Window::Window window;
  window.openWindow("Multi Thread", width, height);
  window.setFrameTimeLowerBound(15000000);
  // initial quads
  std::vector<Quad> quads(quad_count);
  for (auto &quad : quads) {
    auto speed = magnitue_distribution(generator);
    auto angle = angle_distribution(generator);
    quad.position_.x_ = 0.0f;
    quad.position_.y_ = 0.0f;
    quad.position_.z_ = 0.0f;
    quad.velocity_.x_ = cos(angle) * speed;
    quad.velocity_.y_ = sin(angle) * speed;
    quad.position_.z_ = z_distribution(generator);
  }

  CHCEngine::Renderer::Renderer renderer;
  renderer.initializeDevice();
  renderer.setSwapChain(window);
  // uint64_t nano_delta = 1000000;
  auto position_buffer = renderer.getBuffer(
      quad_count, sizeof(Position),
      {{.usage_ = ResourceUsage::RESOURCE_USAGE_SRV}},
      ResourceState::RESOURCE_STATE_COMMON,
      Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC);
  auto vertex_buffer = renderer.getVertexBuffer(
      6, {{"POSITION", DataFormat::DATA_FORMAT_R32G32_FLOAT},
          {"UV", DataFormat::DATA_FORMAT_R32G32_FLOAT}});
  float quaddata[] = {
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 1.0f, 1.0f,  1.0f,  1.0f, 0.0f,
      1.0f,  -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,  0.0f, 0.0f,
  };
  auto initial_copy = renderer.getCopyContext();
  initial_copy->updateBuffer(vertex_buffer, quaddata, sizeof(float) * 24);
  renderer.submitContexts({initial_copy})->waitComplete();

  // pipeline
  Shader ps("quad.hlsl", "PSMain", ShaderType::SHADER_TYPE_PIXEL, true);
  Shader vs("quad.hlsl", "VSMain", ShaderType::SHADER_TYPE_VERTEX, true);
  ShaderSet quad_shader_set(std::vector<Shader>{vs, ps}, {"IndexConstant"});
  auto quad_bind_layout = renderer.getBindLayout(
      quad_shader_set.getBindFormats(BindType::BIND_TYPE_SIT_ALL));
  auto quad_pipeline = renderer.getGraphicsPipeline(
      quad_shader_set,
      {vertex_buffer->getBufferInformation().vetex_attributes_},
      quad_bind_layout);
  Pipeline::Viewport view_port(window.getFrameSize().X,
                               window.getFrameSize().Y);
  Pipeline::Scissor scissor(window.getFrameSize().X, window.getFrameSize().Y);

  auto update = [&](Quad &quad, uint64_t nano_delta) {
    quad.position_.x_ += quad.velocity_.x_ * nano_delta / 10000000;
    quad.position_.y_ += quad.velocity_.y_ * nano_delta / 10000000;
    if (quad.position_.x_ >= 1.0 && quad.velocity_.x_ > 0.0)
      quad.velocity_.x_ *= -1.0;
    if (quad.position_.y_ >= 1.0 && quad.velocity_.y_ > 0.0)
      quad.velocity_.y_ *= -1.0;
    if (quad.position_.x_ <= -1.0 && quad.velocity_.x_ < 0.0)
      quad.velocity_.x_ *= -1.0;
    if (quad.position_.y_ <= -1.0 && quad.velocity_.y_ < 0.0)
      quad.velocity_.y_ *= -1.0;
  };

  auto threadupdate = [&](std::vector<Quad> &quads,
                          std::shared_ptr<Context::CopyContext> context,
                          uint32_t start, uint32_t end, uint64_t nano_delta) {
    std::for_each(quads.begin() + start, quads.begin() + end,
                  std::bind(update, std::placeholders::_1, nano_delta));
    for (uint32_t i = start; i < end; ++i) {
      context->updateBuffer(position_buffer, &quads[i].position_,
                            sizeof(Position), i * sizeof(Position));
    }
  };

  std::mutex queue_mutex;
  std::queue<std::vector<std::shared_ptr<Context::Context>>>
      copy_updates_queue_;

  // at quad update function at window thread
  window.addLoopCallback(
      "Quad Update",
      [&](std::chrono::duration<long long, std::nano> const &delta,
          uint64_t const &frame) {
        std::vector<std::thread> threads(quad_update_thrad_count);
        uint32_t update_size = (quad_count + (quad_update_thrad_count - 1)) /
                               quad_update_thrad_count;
        uint32_t start = 0;
        uint32_t end = update_size;
        std::vector<std::shared_ptr<Context::Context>> copy_contexts;
        for (uint32_t i = 0; i < threads.size(); ++i) {
          auto copy = renderer.getCopyContext();
          copy_contexts.push_back(copy);
          threads[i] = std::thread(threadupdate, std::ref(quads), copy, start,
                                   end, delta.count());
          start += update_size;
          end += update_size;
          end = (std::min)((uint32_t)quads.size(), end);
        }
        for (uint32_t i = 0; i < threads.size(); ++i) {
          if (threads[i].joinable())
            threads[i].join();
        }
        {
          std::lock_guard<std::mutex> lock(queue_mutex);
          copy_updates_queue_.emplace(copy_contexts);
        }
      });
  auto frame_buffer = renderer.getTexture(
      TextureType::TEXTURE_TYPE_2D, RawFormat::RAW_FORMAT_B8G8R8A8, width,
      height, 1, 1,
      {{.usage_ = ResourceUsage::RESOURCE_USAGE_SRV,
        .data_format_ = DataFormat::DATA_FORMAT_B8G8R8A8_UNORM}},
      {{.data_format_ = DataFormat::DATA_FORMAT_B8G8R8A8_UNORM}});

  auto depth_buffer = renderer.getTexture(
      TextureType::TEXTURE_TYPE_2D, RawFormat::RAW_FORMAT_R16, width, height, 1,
      1, {}, {},
      {{.data_format_ = DepthStencilFormat::DEPTH_STENCIL_FORMAT_D16_UNORM}});
  std::vector<std::vector<std::shared_ptr<Context::GraphicsContext>>>
      graphic_contexts(3u);
  std::vector<std::vector<std::shared_ptr<Context::Context>>>
      submit_graphic_contexts(3u);
  std::vector<std::shared_ptr<Context::ContextFence>> graphics_fences(3u);
  std::shared_ptr<Context::ContextFence> copy_fence;
  copy_fence = renderer.getContextFence();
  for (uint32_t i = 0; i < 3; ++i) {
    graphics_fences[i] = renderer.getContextFence();
    for (uint32_t j = 0; j < quad_draw_thrad_count; ++j) {
      graphic_contexts[i].emplace_back(renderer.getGraphicsContext());
      submit_graphic_contexts[i].push_back(graphic_contexts[i].back());
    }
  }
  auto context_record = [&](GraphicsContext *graphic, uint32_t context_index,
                            uint32_t start, uint32_t end,
                            uint32_t swap_chain_index) {
    if (context_index == 0) {
      graphic->clearRenderTarget(renderer.getSwapChainBuffer(swap_chain_index),
                                 {0.1f, 0.6f, 0.7f, 0.0f});
    }
    graphic->setPipeline(quad_pipeline);
    graphic->setViewport(view_port);
    graphic->setScissor(scissor);
    graphic->setRenderTarget(renderer.getSwapChainBuffer(swap_chain_index));
    graphic->setGraphicsBindLayout(quad_bind_layout);
    graphic->bindGraphicsResource(position_buffer, "positions");
    graphic->setVertexBuffers(vertex_buffer);
    graphic->setPrimitiveTopology(
        PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    for (uint32_t i = start; i < end; ++i) {
      graphic->bindGraphicsConstants(&i, 1, "IndexConstant");
      graphic->drawInstanced(6);
    }
    if (context_index == quad_draw_thrad_count - 1) {
      graphic->setSwapChainToPresetState(
          renderer.getSwapChainBuffer(swap_chain_index));
    }
  };
  uint32_t offset =
      (quad_count + (quad_draw_thrad_count - 1)) / quad_draw_thrad_count;
  renderer.addLoopCallback(
      "render loop",
      [&](Renderer &renderer,
          std::chrono::duration<long long, std::nano> const &delta,
          unsigned int swap_chain_index, unsigned long long const &frame) {
        // uint32_t offset = 16;
        uint32_t start = 0;
        uint32_t end = offset;
        for (uint32_t i = 0; i < quad_draw_thrad_count; ++i) {
          graphic_contexts[swap_chain_index][i]
              ->recordCommands<Context::GraphicsContext>(
                  std::bind(context_record, std::placeholders::_1, i, start,
                            end, swap_chain_index),
                  true);
          start += offset;
          end += offset;
          end = (std::min)(end, quad_count);
        }
        std::vector<std::shared_ptr<Context::Context>> copy_contexts;
        {
          std::lock_guard<std::mutex> lock(queue_mutex);
          if (copy_updates_queue_.size()) {
            copy_contexts = copy_updates_queue_.front();
            copy_updates_queue_.pop();
          }
        }
        if (copy_contexts.size()) {

          renderer.submitContexts(copy_fence, copy_contexts);
          renderer.waitFenceSubmitContexts(
              copy_fence, graphics_fences[swap_chain_index],
              submit_graphic_contexts[swap_chain_index]);
        } else {

          renderer.submitContexts(graphics_fences[swap_chain_index],
                                  submit_graphic_contexts[swap_chain_index]);
        }
        renderer.presentSwapChain();
      });

  renderer.waitUntilWindowClose();
  window.waitUntilClose();
  return 0;
}