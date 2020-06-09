#pragma once
#include "../ClassName.h"
namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
struct DepthStencilOpration {
  StencilOperation stencil_fail_operation_ =
      StencilOperation::STENCIL_OPERATION_KEEP;
  StencilOperation stencil_depth_fail_operation_ =
      StencilOperation::STENCIL_OPERATION_KEEP;
  StencilOperation stencil_pass_operation_ =
      StencilOperation::STENCIL_OPERATION_KEEP;
  ComparisonFunction stencil_function_ =
      ComparisonFunction::COMPARISON_FUNCTION_ALWAYS;
};
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine