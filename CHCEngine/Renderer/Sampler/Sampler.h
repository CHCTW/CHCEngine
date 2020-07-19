#pragma once
#include "../ClassName.h"
#include "../DescriptorHeap.h"
#include <numeric>
#include <string>

namespace CHCEngine {
namespace Renderer {
namespace Context {
struct ContextCommand;
}
namespace Sampler {
struct SamplerInformation {
  Filter filter_ = Filter::FILTER_MIN_MAG_MIP_LINEAR;
  TextureAddressMode u_mode_ = TextureAddressMode::TEXTURE_ADDRESS_MODE_CLAMP;
  TextureAddressMode v_mode_ = TextureAddressMode::TEXTURE_ADDRESS_MODE_CLAMP;
  TextureAddressMode w_mode_ = TextureAddressMode::TEXTURE_ADDRESS_MODE_CLAMP;
  Color border_color_ = {0.0f, 0.0f, 0.0f, 0.0f};
  float mip_lod_bias_ = 0.0f;
  unsigned int max_anisotropy_ = 0;
  float min_lod = 0.0f;
  float max_lod = (std::numeric_limits<float>::max)();
  ComparisonFunction comparison_func_ =
      ComparisonFunction::COMPARISON_FUNCTION_NEVER;
};
struct SamplerDescriptorRange {
  std::shared_ptr<DescriptorRange> copy_usage_descriptors_ = nullptr;
  std::shared_ptr<DescriptorRange> bind_usage_descriptors_ = nullptr;
};
class Sampler {
private:
  friend struct Context::ContextCommand;
  SamplerInformation information_;
  SamplerDescriptorRange sampler_descriptor_range_;
  std::string name_;
  GPUDescriptorHandle getSampleHandler(unsigned int index = 0) {
    return sampler_descriptor_range_.bind_usage_descriptors_->getGPUHandle(
        index);
  }

public:
  Sampler(const SamplerInformation &information,
          SamplerDescriptorRange &sampler_descriptor_range);
};
} // namespace Sampler
} // namespace Renderer
} // namespace CHCEngine