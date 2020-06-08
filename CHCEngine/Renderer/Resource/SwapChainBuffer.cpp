#include "../ClassName.h"
#include "SwapChainBuffer.h"
#include "../DescriptorHeap.h"
namespace CHCEngine {
namespace Renderer {
namespace Resource {
CPUDescriptorHandle SwapChainBuffer::getDescriptor() {
  return descriptors_->getHandle(0);
}
}  // namespace Resource
}  // namespace Renderer
}  // namespace CHCEngine