#include "SamplerGroup.h"
namespace CHCEngine {
namespace Renderer {
namespace Sampler {
SamplerGroup::SamplerGroup(SamplerDescriptorRange &sampler_descriptor_range,
                           ComPtr<Device> device)
    : Sampler(sampler_descriptor_range), device_(std::move(device)) {
  samplers_.resize(
      sampler_descriptor_range_.bind_usage_descriptors_->getSize());

}
void SamplerGroup::insertSampler(unsigned int insert_index,
                                 std::shared_ptr<Sampler> sampler) {
  if (insert_index >= samplers_.size()) {
    throw std::exception("Insert index is our of sampler group size!");
  }
  CPUDescriptorHandle handle =
      sampler->getCopySampleHandler(0);
  CPUDescriptorHandle insert_handle =
      sampler_descriptor_range_.bind_usage_descriptors_->getHandle(
          insert_index);
  device_->CopyDescriptorsSimple(1, insert_handle, handle,
                                 D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  samplers_[insert_index] = std::move(sampler);
}
} // namespace Sampler
} // namespace Renderer
} // namespace CHCEngine