#include "../ClassName.h"

#include "Texture.h"
namespace CHCEngine {
namespace Renderer {
namespace Resource {
Texture::Texture(
    ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
    ResourceInformation information, TextureInformation &texture_information,
    std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
        &descriptor_ranges)
    : Resource(gpu_resource, upload_buffer, information, descriptor_ranges),
      texture_information_(std::move(texture_information)) {}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine