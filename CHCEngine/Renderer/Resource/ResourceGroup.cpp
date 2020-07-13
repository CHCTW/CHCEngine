#include "../ClassName.h"

#include "ResourceGroup.h"
namespace CHCEngine {
namespace Renderer {
namespace Resource {
ResourceGroup::ResourceGroup(
    ComPtr<GPUResource> gpu_resource, ResourceInformation information,
    std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
        &descriptor_ranges,
    ComPtr<Device> device)
    : Resource(gpu_resource, nullptr, information, descriptor_ranges),
      device_(std::move(device)) {
  resources_.resize(
      descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]
          ->getSize());
 
}
/*void ResourceGroup::insertResource(
    unsigned int insert_index,std::shared_ptr<Resource> resource,
                                   unsigned int usage_index) {
  if (insert_index >= resources_.size()) {
    throw std::exception("Insert index is our of resource group size!");
  }
  CPUDescriptorHandle handle =
      resource->getCPUCBVSRVUAVUsagebyIndex(usage_index);
  CPUDescriptorHandle insert_handle =
      descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]
          ->getHandle(insert_index);
  device_->CopyDescriptorsSimple(
      1, insert_handle, handle,D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  resources_.emplace(resources_.begin()+insert_index, std::move(resource));
}*/
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine