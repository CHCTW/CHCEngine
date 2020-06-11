#include "../ClassName.h"

#include "DepthStencilSetup.h"

namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
DepthStencilSetup::DepthStencilSetup(DataFormat depth_stencil_formats)
    : depth_stencil_formats_(depth_stencil_formats) {
  depth_enable_ = hasDepthFormat(depth_stencil_formats);
  stencil_enable_ = hasStencilFormat(depth_stencil_formats);
}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine