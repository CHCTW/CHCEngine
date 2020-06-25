#pragma once
#include "Resource.h"

namespace CHCEngine {
namespace Renderer {
namespace Resource {

struct TextureInformation {
  TextureType type_ = TextureType::TEXTURE_TYPE_2D;
  DataFormat data_format_ = DataFormat::DATA_FORMAT_UNKNOWN;
  unsigned long long width_ = 1;
  unsigned int height_ = 1;
  unsigned int depth_ = 1;
  unsigned int mip_levels_ = 1;
};
class Texture : public Resource {
private:
  friend struct Context::ContextCommand;
  TextureInformation texture_information_;

public:
  Texture &operator=(Texture &ref) = delete;
  Texture(
      ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
      ResourceInformation information, TextureInformation texture_information,
      std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
          &descriptor_ranges,
      std::vector<std::pair<DescriptorType, unsigned int>>
          &descriptor_indices);
};
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine