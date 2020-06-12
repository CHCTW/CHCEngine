#pragma once
#include <wrl/client.h>
#include <string_view>

#include "../Resource/Buffer.h"
#include "BindLayout.h"
#include "DepthStencilSetup.h"
#include "Rasterizer.h"
#include "RenderTargetSetup.h"
#include "Shader.h"
#include "Scissor.h"
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
public:
  Pipeline(ComPtr<PipelineState> pipeline_state, PipelineType type);
  void setName(std::string_view name);
};
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine