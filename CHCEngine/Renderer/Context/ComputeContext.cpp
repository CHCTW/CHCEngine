#include "../ClassName.h"

#include "ComputeContext.h"
#include "../Pipeline/Pipeline.h"

namespace CHCEngine {
namespace Renderer {
namespace Context {
void ComputeContext::setPipeline(const std::shared_ptr<Pipeline::Pipeline> &pipeline) {
  if (type_ ==CommandType::COMMAND_TYPE_COMPUTE) {
    if (pipeline->type_ == Pipeline::PipelineType::PIPELINE_TYPE_GRAPHICS) {
      throw std::exception("Only computer pipline can set in compute context");
    }
  }
  context_command_->setPipelineState(pipeline->pipeline_state_);
}
void ComputeContext::setComputeBindLayout(
    const std::shared_ptr<Pipeline::BindLayout> &bind_layout) {
  compute_layout_ = bind_layout;
  context_command_->setComputeBindSignature(bind_layout->bind_signature_);
}
void ComputeContext::bindComputeResource(
    const std::shared_ptr<Resource::Resource> &resource,
    unsigned int usage_index, unsigned int slot_index, BindType bind_type,
    bool direct_bind) {
  context_command_->bindComputeResource(resource, usage_index, slot_index,
                                        bind_type, direct_bind);
}
void ComputeContext::bindComputeResource(
    const std::shared_ptr<Resource::Resource> &resource,
    unsigned int slot_index, unsigned int usage_index) {
  if (!compute_layout_) {
    throw std::exception(
        "Need to set compute bind layout first in this context");
  }
  bool direct_bind = compute_layout_->isDirectBind(slot_index);
  auto type = compute_layout_->getFirstBindType(slot_index);
  bindComputeResource(resource, usage_index, slot_index, type, direct_bind);

}
void ComputeContext::bindComputeResource(
    const std::shared_ptr<Resource::Resource> &resource,
    const std::string &slot_name, unsigned int usage_index) {
  if (!compute_layout_) {
    throw std::exception(
        "Need to set compute bind layout first in this context");
  }
  unsigned int slot_index = compute_layout_->getSlotIndex(slot_name);
  bindComputeResource(resource, slot_index, usage_index);
}
void ComputeContext::setStaticUsageHeap() {
  context_command_->setStaticDescriptorHeap();
}
void ComputeContext::dispatch(unsigned int x, unsigned int y, unsigned int z) {
  context_command_->dispatch(x, y, z);
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine