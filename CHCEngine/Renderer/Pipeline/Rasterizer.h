#pragma once
#include "../ClassName.h"
namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
struct Rasterizer {
  FillMode fill_mode_ = FillMode::FILL_MODE_SOLID;
  CullMode cull_mode_ = CullMode::CULL_MODE_BACK;
  bool front_counter_clock_wise_ = false;
  int depth_bias_ = 0;
  float depth_bias_clamp = 0.0f;
  float slope_scaled_depth_bias_ = 0.0f;
  bool depth_clip_enable_ = true;
  bool multi_sample_enable_ = false;
  bool anti_alaised_line_enable_ = false;
  unsigned int forced_sample_count_ = 0;
  bool conservative_rasterization_enable_ = false;
};
static const Rasterizer default_rasterizer_;
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine