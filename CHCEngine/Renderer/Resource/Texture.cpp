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
                 std::shared_ptr<DescriptorRange> &dsv_range,
                 const std::vector<TextureUsage> &usages,
                 const std::vector<RenderTargetUsage> &render_target_usages,
                 const std::vector<DepthStencilUsage> &depth_stencil_usages)
    : Resource(gpu_resource, upload_buffer, information, resource_desc_range),
      texture_information_(std::move(texture_information)),
      render_target_descriptors_(std::move(rtv_range)),
      depth_stencil_descriptors_(std::move(dsv_range)), usages_(usages),
      render_target_usages_(render_target_usages),
      depth_stencil_usages_(depth_stencil_usages) {}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine