#include "../ClassName.h"

#include "Texture.h"
#include <algorithm>
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
                 const std::vector<DepthStencilUsage> &depth_stencil_usages,
                 ResourceState initial_state)
    : Resource(gpu_resource, upload_buffer, information, resource_desc_range),
      texture_information_(std::move(texture_information)),
      render_target_descriptors_(std::move(rtv_range)),
      depth_stencil_descriptors_(std::move(dsv_range)), usages_(usages),
      render_target_usages_(render_target_usages),
      depth_stencil_usages_(depth_stencil_usages) {
  sub_resource_states_.resize(texture_information.getSubResrouceCount());
  SubResourceState tracking{initial_state, initial_state};
  std::fill(sub_resource_states_.begin(), sub_resource_states_.end(), tracking);
  auto sub_resource_counts_ = this->getSubResrouceCount();
  usage_sub_resources_indices_.resize(usages_.size());
  render_target_sub_resources_indices_.resize(render_target_usages_.size());
  depth_stencil_sub_resources_indices_.resize(depth_stencil_usages_.size());
  for (uint32_t i = 0; i < usages_.size(); ++i) {
    getSubResourceList(usage_sub_resources_indices_[i], usages_[i],
                       texture_information_.mip_levels_,
                       texture_information_.depth_);
    if (usage_sub_resources_indices_[i].size() == sub_resource_counts_) {
      usage_sub_resources_indices_[i] = {all_subresrouce_index};
    }
  }

  for (uint32_t i = 0; i < render_target_usages_.size(); ++i) {
    getRenderTargetSubResourceList(
        render_target_sub_resources_indices_[i], render_target_usages_[i],
        texture_information_.mip_levels_, texture_information_.depth_);
    if (render_target_sub_resources_indices_[i].size() ==
        sub_resource_counts_) {
      render_target_sub_resources_indices_[i] = {all_subresrouce_index};
    }
  }
  for (uint32_t i = 0; i < depth_stencil_usages_.size(); ++i) {
    geDepthStencilSubResourceList(
        depth_stencil_sub_resources_indices_[i], depth_stencil_usages_[i],
        texture_information_.mip_levels_, texture_information_.depth_);
    if (depth_stencil_sub_resources_indices_[i].size() ==
        sub_resource_counts_) {
      depth_stencil_sub_resources_indices_[i] = {all_subresrouce_index};
    }
  }
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine