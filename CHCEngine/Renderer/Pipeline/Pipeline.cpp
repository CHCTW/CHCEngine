#include "../ClassName.h"

#include "Pipeline.h"
namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
Pipeline::Pipeline(ComPtr<PipelineState> pipeline_state, PipelineType type)
    : pipeline_state_(pipeline_state), type_(type) {}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine