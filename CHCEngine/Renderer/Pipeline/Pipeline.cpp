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

std::tuple<uint32_t, uint32_t, uint32_t> Pipeline::getComputeThreadSize() {
  return std::tuple<uint32_t, uint32_t, uint32_t>(
      compute_thread_size_x_, compute_thread_size_y_, compute_thread_size_z_);
}

Pipeline::Pipeline(ComPtr<PipelineState> pipeline_state, PipelineType type)
    : pipeline_state_(pipeline_state), type_(type) {}
Pipeline::Pipeline(ComPtr<PipelineState> pipeline_state, PipelineType type,
                   uint32_t x, uint32_t y, uint32_t z)
    : pipeline_state_(pipeline_state), type_(type), compute_thread_size_x_(x),
      compute_thread_size_y_(y), compute_thread_size_z_(z) {}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine