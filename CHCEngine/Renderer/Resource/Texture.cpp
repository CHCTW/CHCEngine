#include "../ClassName.h"

#include "Texture.h"
namespace CHCEngine {
namespace Renderer {
namespace Resource {
Texture::Texture(
    ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
    ResourceInformation information, TextureInformation texture_information,
    std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
        &descriptor_ranges,
    std::vector<std::pair<DescriptorType, unsigned int>>
        &descriptor_indices)
    : Resource(gpu_resource, upload_buffer, information, descriptor_ranges,
               descriptor_indices),
      texture_information_(texture_information) {}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine