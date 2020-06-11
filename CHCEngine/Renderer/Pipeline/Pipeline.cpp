#include "../ClassName.h"

#include "Pipeline.h"
namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
void Pipeline::setName(std::string_view name) {
  std::string temp(name);
  NAME_D3D12_OBJECT_STRING(pipeline_state_, temp);
  name_ = name;
}
Pipeline::Pipeline(ComPtr<PipelineState> pipeline_state, PipelineType type)
    : pipeline_state_(pipeline_state), type_(type) {}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine