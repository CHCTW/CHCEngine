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
ResourceGroup::ResourceGroup(ComPtr<GPUResource> gpu_resource,
                             ResourceInformation information,
                             ResourceDescriptorRange &resource_desc_range,
                             ComPtr<Device> device)
    : Resource(gpu_resource, nullptr, information, resource_desc_range),
      device_(std::move(device)) {
  resources_.resize(
      resource_descriptor_range_.bind_usage_descriptors_->getSize());
  usage_indices_.resize(
      resource_descriptor_range_.bind_usage_descriptors_->getSize());
}
void ResourceGroup::insertResource(unsigned int insert_index,
                                   std::shared_ptr<Resource> resource,
                                   unsigned int usage_index) {
  if (insert_index >= resources_.size()) {
    throw std::exception("Insert index is out of resource group size!");
  }
  if (resource->getType() == ResourceType::RESOURCE_TYPE_GROUP) {
    throw std::exception("Can't Insert a resource group to resource group!");
  }
  CPUDescriptorHandle handle =
      resource->getCPUCBVSRVUAVUsagebyIndex(usage_index);
  CPUDescriptorHandle insert_handle =
      resource_descriptor_range_.bind_usage_descriptors_->getHandle(
          insert_index);
  device_->CopyDescriptorsSimple(1, insert_handle, handle,
                                 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  resources_[insert_index] = std::move(resource);
  usage_indices_[insert_index] = usage_index;
}
const std::shared_ptr<Resource> &
ResourceGroup::getResrouce(uint32_t index) const {
  if (index >= resources_.size()) {
    throw std::exception("Index is out of resource group size!");
  }
  if (!resources_[index]) {
    std::string error("Try to access empty resource in resource group, at "
                      "index :  ");
    error += std::to_string(index);
    throw std::exception(error.c_str());
  }
  return resources_[index];
}
uint32_t ResourceGroup::getUsageIndex(uint32_t index) const {
  if (index >= resources_.size()) {
    throw std::exception("Index is out of resource group size!");
  }
  if (!resources_[index]) {
    std::string error(
        "Try to access empty resource usage index in resource group, at "
        "index :  ");
    error += std::to_string(index);
    throw std::exception(error.c_str());
  }
  return usage_indices_[index];
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine