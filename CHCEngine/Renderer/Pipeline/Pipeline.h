#pragma once
#include <string_view>
#include <tuple>
#include <wrl/client.h>

#include "../Resource/Buffer.h"
#include "BindLayout.h"
#include "DepthStencilSetup.h"
#include "Rasterizer.h"
#include "RenderTargetSetup.h"
#include "Scissor.h"
#include "Shader.h"
#include "Viewport.h"

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Context {
class GraphicsContext;
class ComputeContext;
} // namespace Context
namespace Pipeline {
struct SampleSetup {
  unsigned int count_ = 1;
  unsigned int quality_ = 0;
};
static const SampleSetup defualt_sample_setup;
enum class PipelineType { PIPELINE_TYPE_GRAPHICS, PIPELINE_TYPE_COMPUTE };
// goint to have state change here
class Pipeline {
private:
  std::string name_;
  friend class Context::GraphicsContext;
  friend class Context::ComputeContext;
  PipelineType type_;
  ComPtr<PipelineState> pipeline_state_;
  uint32_t compute_thread_size_x_ = 0;
  uint32_t compute_thread_size_y_ = 0;
  uint32_t compute_thread_size_z_ = 0;

public:
  Pipeline(ComPtr<PipelineState> pipeline_state, PipelineType type);
  Pipeline(ComPtr<PipelineState> pipeline_state, PipelineType type, uint32_t x,
           uint32_t y, uint32_t z);
  void setName(std::string_view name);
  std::tuple<uint32_t, uint32_t, uint32_t> getComputeThreadSize();
};
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine