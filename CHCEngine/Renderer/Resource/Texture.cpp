#include "../ClassName.h"

#include "Texture.h"
namespace CHCEngine {
namespace Renderer {
namespace Resource {
Texture::Texture(ComPtr<GPUResource> gpu_resource,
                 ComPtr<GPUResource> upload_buffer,
                 ResourceInformation information,
                 TextureInformation &texture_information,
                 ResourceDescriptorRange &resource_desc_range,
                 std::shared_ptr<DescriptorRange> &rtv_range,
                 std::shared_ptr<DescriptorRange> &dsv_range)
    : Resource(gpu_resource, upload_buffer, information, resource_desc_range),
      texture_information_(std::move(texture_information)),
      render_target_usage_(std::move(rtv_range)),
      depth_stencil_usage_(std::move(dsv_range)) {}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine