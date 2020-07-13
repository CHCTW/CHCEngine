#include "../ClassName.h"

#include "../D3D12Utilities.hpp"

#include "Resource.h"
//#include "../Pipeline/BindLayout.h"

namespace CHCEngine {
namespace Renderer {
namespace Resource {
Resource::Resource(ComPtr<GPUResource> gpu_resource,
                   ComPtr<GPUResource> upload_buffer, const std::string &name,
                   ResourceType type, ResourceUpdateType usage)
    : gpu_resource_(gpu_resource),
      upload_buffer_(upload_buffer), information_{name, type, usage} {
  if (upload_buffer_) {
    D3D12_RANGE readRange{0, 0};
    ThrowIfFailed(
        upload_buffer_->Map(0, &readRange, &upload_buffer_map_pointer_));
  }
}
Resource::Resource(ComPtr<GPUResource> gpu_resource,
                   ComPtr<GPUResource> upload_buffer,
                   ResourceInformation information)
    : gpu_resource_(gpu_resource),
      upload_buffer_(upload_buffer), information_{information} {
  if (upload_buffer_) {
    D3D12_RANGE readRange{0, 0};
    ThrowIfFailed(
        upload_buffer_->Map(0, &readRange, &upload_buffer_map_pointer_));
  }
}
Resource::Resource(
    ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
    ResourceInformation information,
    std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
        &descriptor_ranges)
    : gpu_resource_(gpu_resource),
      upload_buffer_(upload_buffer), information_{information},
      descriptor_ranges_(std::move(descriptor_ranges)) {
  if (upload_buffer_) {
    D3D12_RANGE readRange{0, 0};
    ThrowIfFailed(
        upload_buffer_->Map(0, &readRange, &upload_buffer_map_pointer_));
  }
}
Resource::Resource(
    ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
    ResourceInformation information,
                   ResourceDescriptorRange &resource_desc_range)
    : gpu_resource_(gpu_resource),
      upload_buffer_(upload_buffer), information_{information},
      resource_descriptor_range_(std::move(resource_desc_range)) {
  if (upload_buffer_) {
    D3D12_RANGE readRange{0, 0};
    ThrowIfFailed(
        upload_buffer_->Map(0, &readRange, &upload_buffer_map_pointer_));
  }
}
/*CPUDescriptorHandle Resource::getRTVHandleByUsageIndex(unsigned int index) {
  if (index >=
      descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_RTV]->getSize()) {
    throw std::exception("Invalid usage index, out of rtv usage indices size");
  }
  return descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_RTV]->getHandle(
      index);
}
CPUDescriptorHandle Resource::getDSVHandleByUsageIndex(unsigned int index) {
  if (index >=
      descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_DSV]->getSize()) {
    throw std::exception("Invalid usage index, out of dsv usage indices size");
  }
  return descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_DSV]->getHandle(
      index);
}*/
GPUDescriptorHandle Resource::getCBVSRVUAVUsagebyIndex(unsigned int index) {
  if (index >= descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]
                   ->getSize()) {
    throw std::exception("Invalid usage index, out of usage indices size");
  }
  return descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]
      ->getGPUHandle(index);
}
CPUDescriptorHandle Resource::getCPUCBVSRVUAVUsagebyIndex(unsigned int index) {
  if (index >= descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]
                   ->getSize()) {
    throw std::exception("Invalid usage index, out of usage indices size");
  }
  return descriptor_ranges_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV]
      ->getHandle(index);
}
void Resource::setName(std::string_view name) {
  std::string temp(name);
  NAME_D3D12_OBJECT_STRING(gpu_resource_, temp);
  information_.name_ = name;
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine