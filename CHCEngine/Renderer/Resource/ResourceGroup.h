#pragma once
#include <vector>

#include "../ClassName.h"
#include "Resource.h"
namespace CHCEngine {
namespace Renderer {
namespace Resource {
class ResourceGroup : public Resource {
private:
  ComPtr<Device> device_;
  std::vector<std::shared_ptr<Resource>> resources_;
public:
  ResourceGroup(ComPtr<GPUResource> gpu_resource,
                ResourceInformation information,
                ResourceDescriptorRange &resource_desc_range ,
      ComPtr<Device> device_);
  void insertResource(unsigned int insert_index,std::shared_ptr<Resource> resource, unsigned int usage_index = 0);
};
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine