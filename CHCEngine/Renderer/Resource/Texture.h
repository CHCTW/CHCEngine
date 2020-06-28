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
};
class Texture : public Resource {
private:
  friend struct Context::ContextCommand;
  TextureInformation texture_information_;
  friend class Context::CopyContext;
public:
  Texture &operator=(Texture &ref) = delete;
  Texture(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
          ResourceInformation information,
          TextureInformation &texture_information,
          std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
              &descriptor_ranges);
  const TextureInformation & getTextureInformation() const {
    return texture_information_;
  }
};
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine