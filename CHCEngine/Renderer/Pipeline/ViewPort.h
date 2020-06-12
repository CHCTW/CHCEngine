#pragma once
#include "../ClassName.h"

namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
struct Viewport : public ViewportBase {
  Viewport(float width, float height)
      : ViewportBase{0.0f,
                     0.0f,
                     width,
                     height,
                     default_min_depth_,
                     default_max_depth_} {}
  Viewport(int width, int height)
      : ViewportBase{0.0f,
                     0.0f,
                     static_cast<float>(width),
                     static_cast<float>(height),
                     default_min_depth_,
                     default_max_depth_} {}

  /* Viewport(float left_x, float top_y, float width, float height)
       : left_x_(left_x), top_y_(top_y), width_(width), height_(height) {}*/
  Viewport(int left_x, int top_y, int width, int height)
      : ViewportBase{static_cast<float>(left_x), static_cast<float>(top_y),
                     static_cast<float>(width),  static_cast<float>(height),
                     default_min_depth_,         default_max_depth_} {}
}; // namespace Pipeline
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine