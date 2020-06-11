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
class DepthStencilSetup {
private:
  bool depth_enable_ = false;
  DepthWriteMask depth_write_mask_ = DepthWriteMask::DEPTH_WRITE_MASK_ALL;
  ComparisonFunction depth_comparison_function_ =
      ComparisonFunction::COMPARISON_FUNCTION_LESS;
  bool stencil_enable_ = false;
  unsigned char stencil_read_mask_ = default_stencil_read_mask_;
  unsigned char stencil_write_mask_ = default_stencil_write_mask_;
  DepthStencilOpration front_face_;
  DepthStencilOpration back_face_;
  DataFormat depth_stencil_formats_ = DataFormat::DATA_FORMAT_UNKNOWN;

public:
  DepthStencilSetup() = default;
  // this will use the foramt to auto turn on/off depth/stencil
  DepthStencilSetup(DataFormat depth_stencil_formats);
  void setDepthEnable(bool depth_enable) { depth_enable_ = depth_enable; }
  void setStencilEnable(bool stencil_enable) {
    stencil_enable_ = stencil_enable;
  }
  void setDepthComparisonFunction(ComparisonFunction comparison) {
    depth_comparison_function_ = comparison;
  }
  bool isDepthEnable() const { return depth_enable_; }
  DepthWriteMask getDepthWriteMask() const { return depth_write_mask_; }
  ComparisonFunction getDepthComparisonFunction() const {
    return depth_comparison_function_;
  }
  bool isStencilEnable() const { return stencil_enable_; }
  unsigned char getStencilReadMask() const { return stencil_read_mask_; }
  unsigned char getStencilWriteMask() const { return stencil_write_mask_; }
  DepthStencilOpration getFrontFaceOpeartion() const { return front_face_; }
  DepthStencilOpration getBackFaceOpeartion() const { return back_face_; }
  DataFormat getFormat() const { return depth_stencil_formats_; }
};
static const DepthStencilSetup default_depth_stencil_setup_;
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine