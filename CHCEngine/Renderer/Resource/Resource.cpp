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
        &descriptor_ranges,
    std::vector<std::pair<DescriptorType, unsigned int>>
        &usage_indices)
    : gpu_resource_(gpu_resource),
      upload_buffer_(upload_buffer), information_{information},
      descriptor_ranges_(std::move(descriptor_ranges)), usage_indices_(std::move(usage_indices)) {
  if (upload_buffer_) {
    D3D12_RANGE readRange{0, 0};
    ThrowIfFailed(
        upload_buffer_->Map(0, &readRange, &upload_buffer_map_pointer_));
  }
}
CPUDescriptorHandle Resource::getCPUHandleByUsageIndex(unsigned int index) {
  if (index>=usage_indices_.size()) {
      throw std::exception(
          "Invalid usage index, out of usage indices size");
  }
  const auto &p = usage_indices_[index];
  return descriptor_ranges_[p.first]->getHandle(p.second);
}
GPUDescriptorHandle Resource::getGPUHandleByUsageIndex(unsigned int index) {
  if (index >= usage_indices_.size()) {
    throw std::exception("Invalid usage index, out of usage indices size");
  }
  const auto &p = usage_indices_[index];
  return descriptor_ranges_[p.first]->getGPUHandle(p.second);
}
void Resource::setName(std::string_view name) {
  std::string temp(name);
  NAME_D3D12_OBJECT_STRING(gpu_resource_, temp);
  information_.name_ = name;
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine