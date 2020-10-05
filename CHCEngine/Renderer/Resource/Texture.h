#pragma once
#include "Resource.h"

namespace CHCEngine {
namespace Renderer {
namespace Resource {
struct TextureInformation {
  TextureType type_ = TextureType::TEXTURE_TYPE_2D;
  RawFormat data_format_ = RawFormat::RAW_FORMAT_UNKNOWN;
  unsigned long long width_ = 1;
  unsigned int height_ = 1;
  unsigned int depth_ = 1;
  unsigned int mip_levels_ = 1;
  std::vector<TextureFootPrint> foot_prints_;
  std::vector<unsigned int> row_counts_;
  std::vector<unsigned long long> row_byte_sizes_;
  unsigned long long byte_size_;
  unsigned int getSubResrouceCount() const {
    if (type_ == TextureType ::TEXTURE_TYPE_3D)
      return mip_levels_;
    return mip_levels_ * depth_;
  }
};
class Texture : public Resource {
private:
  friend struct Context::ContextCommand;
  TextureInformation texture_information_;
  friend class Context::CopyContext;
  std::shared_ptr<DescriptorRange> render_target_descriptors_;
  std::shared_ptr<DescriptorRange> depth_stencil_descriptors_;
  std::vector<TextureUsage> usages_;
  std::vector<RenderTargetUsage> render_target_usages_;
  std::vector<DepthStencilUsage> depth_stencil_usages_;

public:
  Texture &operator=(Texture &ref) = delete;
  Texture(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
          ResourceInformation information,
          TextureInformation &texture_information,
          ResourceDescriptorRange &resource_desc_range,
          std::shared_ptr<DescriptorRange> &rtv_range,
          std::shared_ptr<DescriptorRange> &dsv_range,
          const std::vector<TextureUsage> &usages,
          const std::vector<RenderTargetUsage> &render_target_usages,
          const std::vector<DepthStencilUsage> &depth_stencil_usages,
          ResourceState initial_state);
  const TextureInformation &getTextureInformation() const {
    return texture_information_;
  }
};
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine