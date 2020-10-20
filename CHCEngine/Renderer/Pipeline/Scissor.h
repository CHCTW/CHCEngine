#pragma once
#include "../ClassName.h"

namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
struct Scissor : ScissorBase {
  Scissor(long right, long bottom) : ScissorBase{0, 0, right, bottom} {}
  Scissor() {}
};
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine