#pragma once
#include <vector>

#include "../ClassName.h"
#include "Resource.h"
namespace CHCEngine {
namespace Renderer {
namespace Context {
struct ContextCommand;
}
namespace Resource {
class ResourceGroup : public Resource {
  friend struct Context::ContextCommand;

private:
  ComPtr<Device> device_;
  std::vector<std::shared_ptr<Resource>> resources_;
  std::vector<uint32_t> usage_indices_;
  std::shared_ptr<Resource> getResource(unsigned int index);

public:
  ResourceGroup(ComPtr<GPUResource> gpu_resource,
                ResourceInformation information,
                ResourceDescriptorRange &resource_desc_range,
                ComPtr<Device> device);
  void insertResource(unsigned int insert_index,
                      std::shared_ptr<Resource> resource,
                      unsigned int usage_index = 0);
  const std::shared_ptr<Resource> &getResrouce(uint32_t index) const;
  uint32_t getUsageIndex(uint32_t index) const;
};
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine