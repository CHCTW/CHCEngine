#include "../ClassName.h"

#include "Sampler.h"
namespace CHCEngine {
namespace Renderer {
namespace Sampler {
Sampler::Sampler(SamplerDescriptorRange &sampler_descriptor_range)
    : sampler_descriptor_range_(std::move(sampler_descriptor_range)) {}
Sampler::Sampler(const SamplerInformation &information,
                 SamplerDescriptorRange &sampler_descriptor_range)
    : information_(information),
      sampler_descriptor_range_(std::move(sampler_descriptor_range)) {}
} // namespace Sampler
} // namespace Renderer
} // namespace CHCEngine