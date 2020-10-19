#include "CHCEngine.h"
#include "QuadHlslCompat.h"
#include <algorithm>
#include <chrono>
#include <functional>
#include <mutex>
#include <queue>
#include <random>

const static uint32_t width = 800;
const static uint32_t height = 600;
const static uint32_t quad_count = 5000;
const static uint32_t quad_update_thrad_count = 14;
const static uint32_t quad_draw_thrad_count = 14;

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
std::uniform_real_distribution<float> magnitue_distribution(0.0001f, 0.0005f);
std::uniform_real_distribution<float> z_distribution(0.0f, 0.9999f);
std::uniform_real_distribution<float> xy_distribution(-0.9999f, 0.9999f);
std::uniform_real_distribution<float> color_distribution(0.0f, 0.9999f);

int main() {

  CHCEngine::Window::Window window;
  window.openWindow("Multi Thread", width, height);
  // window.setFrameTimeLowerBound(30000000);
  // initial quads
  std::vector<Quad> quads(quad_count);
  std::vector<CircleColor> colors(quad_count);
  uint32_t i = 0;

  for (auto &quad : quads) {
    auto speed = magnitue_distribution(generator);
    auto angle = angle_distribution(generator);
    quad.position_.x_ = 0.0f;
    quad.position_.y_ = 0.0f;
    quad.position_.z_ = 0.0f;
    quad.velocity_.x_ = cos(angle) * speed;
    quad.velocity_.y_ = sin(angle) * speed;
    quad.position_.z_ = z_distribution(generator);
    colors[i].r_ = color_distribution(generator);
    colors[i].g_ = color_distribution(generator);
    colors[i].b_ = color_distribution(generator);
    ++i;
  }

  CHCEngine::Renderer::Renderer renderer;
  renderer.initializeDevice();
  renderer.setSwapChain(window);
  // uint64_t nano_delta = 1000000;

  // find out why there are inconsit update issue
  // should when use dynamic resourse, only have one
  // upload buffer, however, since we could record several
  // update buffe commands, which means the data in upload
  // buffer can be correpted,
  // using static will always allocate new chunck, so it's fine
  // but since every same type context use the same upload buffer
  // it will be slow, I am going to give every context a dynamic
  // upload buffer, dynamic resource will have same upload buffer count
  // with swap chain
  auto position_buffer = renderer.getBuffer(
      quad_count, sizeof(Position),
      {{.usage_ = ResourceUsage::RESOURCE_USAGE_SRV}},
      ResourceState::RESOURCE_STATE_COMMON,
      Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC);
  auto color_buffer = renderer.getBuffer(
      quad_count, sizeof(CircleColor),
      {{.usage_ = ResourceUsage::RESOURCE_USAGE_SRV}},
      ResourceState::RESOURCE_STATE_COMMON,
      Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_STATIC);

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
  initial_copy->updateBuffer(color_buffer, colors.data(),
                             sizeof(CircleColor) * quad_count);
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
      quad_bind_layout, Pipeline::default_render_setup_,
      Pipeline::depth_16_write_stencil_setup_);
  Pipeline::Viewport view_port(window.getFrameSize().X,
                               window.getFrameSize().Y);
  Pipeline::Scissor scissor(window.getFrameSize().X, window.getFrameSize().Y);

  auto update = [&](Quad &quad, uint64_t nano_delta) {
    quad.position_.x_ += quad.velocity_.x_ * nano_delta / 1000000;
    quad.position_.y_ += quad.velocity_.y_ * nano_delta / 1000000;
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
        uint32_t update_size = quad_count / quad_update_thrad_count;
        uint32_t add = quad_count % quad_update_thrad_count;
        uint32_t start = 0;
        std::vector<std::shared_ptr<Context::Context>> copy_contexts;
        for (uint32_t i = 0; i < threads.size(); ++i) {
          auto copy = renderer.getCopyContext();
          copy_contexts.push_back(copy);
          uint32_t end = start + update_size;
          if (i < add)
            ++end;
          threads[i] = std::thread(threadupdate, std::ref(quads), copy, start,
                                   end, delta.count());
          start = end;
          // end = (std::min)((uint32_t)quads.size(), end);
        }
        for (uint32_t i = 0; i < threads.size(); ++i) {
          if (threads[i].joinable())
            threads[i].join();
        }
        {
          std::lock_guard<std::mutex> lock(queue_mutex);
          if (copy_updates_queue_.size() > 3) {
            copy_updates_queue_.pop();
          }
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
  std::vector<std::shared_ptr<Context::ContextFence>> copy_fences(3u);
  std::vector<std::vector<std::shared_ptr<Context::Context>>> copy_contexts(3u);

  for (uint32_t i = 0; i < 3; ++i) {
    graphics_fences[i] = renderer.getContextFence();
    copy_fences[i] = renderer.getContextFence();
    for (uint32_t j = 0; j < quad_draw_thrad_count; ++j) {
      graphic_contexts[i].emplace_back(renderer.getGraphicsContext());
      submit_graphic_contexts[i].push_back(graphic_contexts[i].back());
    }
  }
  auto context_record = [&](GraphicsContext *graphic, uint32_t context_index,
                            uint32_t start, uint32_t end,
                            uint32_t swap_chain_index) {
    try {

      if (context_index == 0) {
        graphic->clearRenderTarget(
            renderer.getSwapChainBuffer(swap_chain_index),
            {0.0f, 0.0f, 0.0f, 0.0f});
        graphic->clearDepthStencil(depth_buffer);
      }
      graphic->setPipeline(quad_pipeline);
      graphic->setViewport(view_port);
      graphic->setScissor(scissor);
      graphic->setRenderTarget(renderer.getSwapChainBuffer(swap_chain_index),
                               depth_buffer);
      graphic->setGraphicsBindLayout(quad_bind_layout);
      graphic->bindGraphicsResource(position_buffer, "positions");
      graphic->bindGraphicsResource(color_buffer, "colors");
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
    } catch (std::exception &e) {
      std::cerr << "exception caught: " << e.what() << '\n';
    }
  };
  auto before = std::chrono::high_resolution_clock::now();
  auto deltas = std::chrono::high_resolution_clock::now() - before;
  uint32_t offset = quad_count / quad_draw_thrad_count;
  uint32_t plus = quad_count % quad_draw_thrad_count;
  renderer.addLoopCallback(
      "render loop",
      [&](Renderer &renderer,
          std::chrono::duration<long long, std::nano> const &delta,
          unsigned int swap_chain_index, unsigned long long const &frame) {
        uint32_t start = 0;
        try {
          // record next 2 frame record
          // take advantage with multi threading record
          // when submit context, it will wait until the
          // record done,
          // we can have cpu record and gpu work overlap
          // put record before submit, buy a little time since we might
          // have to copy context from queue, high latency but low frame time
          // approach
          uint32_t next = (swap_chain_index + 2u) % 3u;
          for (uint32_t i = 0; i < quad_draw_thrad_count; ++i) {
            uint32_t end = start + offset;
            if (i < plus)
              ++end;
            graphic_contexts[next][i]->recordCommands<Context::GraphicsContext>(
                std::bind(context_record, std::placeholders::_1, i, start, end,
                          next),
                true);
            start = end;
          }
          // std::vector<std::shared_ptr<Context::Context>> copy_contexts;

          {
            std::lock_guard<std::mutex> lock(queue_mutex);
            // std::cout << copy_updates_queue_.size() << std::endl;
            if (copy_updates_queue_.size()) {
              copy_contexts[next] = copy_updates_queue_.front();
              copy_updates_queue_.pop();
            }
          }
          if (copy_contexts[next].size()) {
            renderer.waitFenceSubmitContexts(graphics_fences[next],
                                             copy_fences[swap_chain_index],
                                             copy_contexts[next]);
            copy_contexts[swap_chain_index].clear();

            renderer.waitFenceSubmitContexts(
                copy_fences[swap_chain_index],
                graphics_fences[swap_chain_index],
                submit_graphic_contexts[swap_chain_index]);

          } else {
            renderer.waitFenceSubmitContexts(
                graphics_fences[next], graphics_fences[swap_chain_index],
                submit_graphic_contexts[swap_chain_index]);
          }
          renderer.presentSwapChain();
        } catch (std::exception &e) {
          std::cerr << "exception caught: " << e.what() << '\n';
        }
      });

  window.waitUntilClose();
  renderer.waitUntilWindowClose();
  return 0;
}