#include "../ClassName.h"

#include "ComputeContext.h"

namespace CHCEngine {
namespace Renderer {
namespace Context {
void ComputeContext::setPipeline(std::shared_ptr<Pipeline::Pipeline> pipeline) {
  if (type_ ==CommandType::COMMAND_TYPE_COMPUTE) {
    if (pipeline->type_ == Pipeline::PipelineType::PIPELINE_TYPE_GRAPHICS) {
      throw std::exception("Only computer pipline can set in compute context");
    }
  }
  context_command_->setPipelineState(pipeline->pipeline_state_);
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine