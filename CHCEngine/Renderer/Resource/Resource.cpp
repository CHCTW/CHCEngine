#include "../ClassName.h"
#include "../D3D12Utilities.hpp"

#include "Resource.h"
//#include "../Pipeline/BindLayout.h"

namespace CHCEngine {
namespace Renderer {
namespace Resource {
Resource::Resource(ComPtr<GPUResource> gpu_resource,
                   ComPtr<GPUResource> upload_buffer, const std::string &name,
                   ResourceType type, ResourceUsage usage)
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
void Resource::setName(std::string_view name) {
  std::string temp(name);
  NAME_D3D12_OBJECT_STRING(gpu_resource_, temp);
  information_.name_ = name;
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine