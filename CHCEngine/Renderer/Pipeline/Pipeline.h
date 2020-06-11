#pragma once
#include <wrl/client.h>

#include "../Resource/Buffer.h"
#include "BindLayout.h"
#include "DepthStencilSetup.h"
#include "Rasterizer.h"
#include "RenderTargetSetup.h"
#include "Shader.h"

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
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
  PipelineType type_;
  ComPtr<PipelineState> pipeline_state_;

public:
  Pipeline(ComPtr<PipelineState> pipeline_state, PipelineType type);
};
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine