#pragma once
#include "../ClassName.h"

namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
struct BlendState {
  bool enable_ = false;
  bool logic_operation_ebale_ = false;
  Blend source_blend_ = Blend::BLEND_ONE;
  Blend destiny_blend_ = Blend::BLEND_ZERO;
  BlendOperation blend_operation_ = BlendOperation::BLEND_OPERATION_ADD;
  Blend source_alpha_blend_ = Blend::BLEND_ONE;
  Blend destiny_alpha_blend_ = Blend::BLEND_ZERO;
  BlendOperation blend_alpha_operation_ = BlendOperation::BLEND_OPERATION_ADD;
  LogicOperation logic_operation_ = LogicOperation::LOGIC_OPERATION_NOOP;
  ColorWriteMask write_mask_ = ColorWriteMask::COLOR_WRITE_MASK_ALL;
};
class RenderTargetSetup {
private:
  unsigned size_ = 0;
  bool alpha_to_coverage_enable_ = false;
  bool independent_blend_enable = false;
  BlendState blend_states_[render_targets_limits_];
  DataFormat render_formats_[render_targets_limits_] = {
      DataFormat::DATA_FORMAT_UNKNOWN};

public:
  RenderTargetSetup(const std::vector<DataFormat> &formats);
  RenderTargetSetup(
      const std::vector<std::pair<DataFormat, BlendState>> &formats);
  RenderTargetSetup(unsigned int count, const DataFormat &formats);
  RenderTargetSetup(unsigned int count,
                    const std::pair<DataFormat, BlendState> &formats);
  void setAlphaToCoverage(bool coverage) {
    alpha_to_coverage_enable_ = coverage;
  }
  void setIndependentBlend(bool blend) { independent_blend_enable = blend;
  }
  void setFormat(unsigned int index, const DataFormat &formats);
  void setBlendState(unsigned int index, const BlendState &state);
};
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine