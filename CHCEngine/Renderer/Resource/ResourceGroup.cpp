#include "../ClassName.h"

#include "ResourceGroup.h"
namespace CHCEngine {
namespace Renderer {
namespace Resource {
std::shared_ptr<Resource> ResourceGroup::getResource(unsigned int index) {
  if (index >= resources_.size()) {
    throw std::exception("Index is our of resource group size!");
  }
  return resources_[index];
}
ResourceGroup::ResourceGroup(
    ComPtr<GPUResource> gpu_resource, ResourceInformation information,
                             ResourceDescriptorRange &resource_desc_range,
    ComPtr<Device> device)
    : Resource(gpu_resource, nullptr, information, resource_desc_range),
      device_(std::move(device)) {
  resources_.resize(resource_descriptor_range_.bind_usage_descriptors_
          ->getSize());
 
}
void ResourceGroup::insertResource(
    unsigned int insert_index,std::shared_ptr<Resource> resource,
                                   unsigned int usage_index) {
  if (insert_index >= resources_.size()) {
    throw std::exception("Insert index is our of resource group size!");
  }
  CPUDescriptorHandle handle =
      resource->getCPUCBVSRVUAVUsagebyIndex(usage_index);
  CPUDescriptorHandle insert_handle =
      resource_descriptor_range_.bind_usage_descriptors_->getHandle(insert_index);
  device_->CopyDescriptorsSimple(
      1, insert_handle, handle,D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  resources_[insert_index]=std::move(resource);
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine