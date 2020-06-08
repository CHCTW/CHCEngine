#pragma once
#include <memory>

#include "Resource.h"
namespace CHCEngine {
namespace Renderer {
class DescriptorRange;
namespace Resource {
// only swap chain will have this simple way that can store the desc handle
// in it self
class SwapChainBuffer : public Resource {
  friend class Renderer;
 private:
  unsigned int width_;
  unsigned int height_;
  std::shared_ptr<DescriptorRange> descriptors_;
  SwapChainBuffer(ComPtr<GPUResource> gpu_resource,
                  std::shared_ptr<DescriptorRange> descriptors,
                  const std::string& name, unsigned int width,
                  unsigned int height)
      : Resource(gpu_resource, name, ResourceType::RESOURCE_TYPE_SWAP_CHAIN,
                 ResourceUsage::RESOURCE_USAGE_NONE),
        descriptors_(descriptors),
        width_(width),
        height_(height) {}
 public:
  //SwapChainBuffer() = delete;
  SwapChainBuffer(SwapChainBuffer& ref) = default;
  SwapChainBuffer(SwapChainBuffer&& ref) = default;
  CPUDescriptorHandle getDescriptor();
};
}  // namespace Resource
}  // namespace Renderer
}  // namespace CHCEngine