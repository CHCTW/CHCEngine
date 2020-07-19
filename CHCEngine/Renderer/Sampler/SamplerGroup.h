#pragma once
#include "Sampler.h"
#include <memory>
#include <vector>
namespace CHCEngine {
namespace Renderer {
namespace Sampler {
class SamplerGroup : public Sampler {
protected:
  friend struct Context::ContextCommand;
  std::vector<std::shared_ptr<Sampler>> samplers_;
  ComPtr<Device> device_;

public:
  SamplerGroup(SamplerDescriptorRange &sampler_descriptor_range,
               ComPtr<Device> devic);
  void insertSampler(unsigned int insert_index,
                      std::shared_ptr<Sampler> samplere);
};
} // namespace Sampler
} // namespace Renderer
} // namespace CHCEngine