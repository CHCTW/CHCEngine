#include "../ClassName.h"

#include "../Pipeline/Pipeline.h"
#include "../Resource/ResourceGroup.h"
#include "ComputeContext.h"

namespace CHCEngine {
namespace Renderer {
namespace Context {
void ComputeContext::setPipeline(
    const std::shared_ptr<Pipeline::Pipeline> &pipeline) {
  if (type_ == CommandType::COMMAND_TYPE_COMPUTE) {
    if (pipeline->type_ == Pipeline::PipelineType::PIPELINE_TYPE_GRAPHICS) {
      throw std::exception("Only computer pipeline can set in compute context");
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
void ComputeContext::updateBindingResrouceState(
    const std::shared_ptr<Resource::Resource> &resource, uint32_t slot_index,
    uint32_t usage_index, const std::shared_ptr<Pipeline::BindLayout> &layout) {
  auto &slot = layout->getBindSlot(slot_index);
  // won't  update if there is unbound binding
  if (slot.isUnbound)
    return;
  if (resource->getType() == Resource::ResourceType::RESOURCE_TYPE_GROUP) {
    /* const std::shared_ptr<Resource::ResourceGroup> group =
         std::static_pointer_cast<Resource::ResourceGroup>(resource);
     uint32_t group_index = 0;
     for (auto &format : slot.formats_) {
       for (uint32_t i = 0; i < format.resource_count_; ++i) {
         ++group_index;
       }
     }*/
  }
}
/*void ComputeContext::bindComputeResource(
    const std::shared_ptr<Resource::Resource> &resource,
    unsigned int usage_index, unsigned int slot_index,
    const Pipeline::BindSlot &slot) {

}*/
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
  flushBarriers();
  context_command_->dispatch(x, y, z);
}
void ComputeContext::resourceTransition(
    const std::shared_ptr<Resource::Resource> &resource,
    ResourceState before_state, ResourceState after_state, bool set_barrier,
    ResourceTransitionFlag flag, unsigned int subresource_index) {
  transitions_.push_back(
      {resource, before_state, after_state, flag, subresource_index});
  if (set_barrier) {
    flushBarriers();
  }
}
void ComputeContext::resourceTransitionTo(
    const std::shared_ptr<Resource::Resource> &resource,
    ResourceState after_state, bool split_transition,
    uint32_t subresource_index, bool set_barrier) {
  updateContextResourceState(resource, after_state, split_transition,
                             subresource_index);
  if (set_barrier)
    flushBarriers();
}
void ComputeContext::uavResourceWait(
    const std::shared_ptr<Resource::Resource> &resource, bool set_barrier) {
  UAVWait wait;
  wait.resource_ = resource;
  uav_waits_.emplace_back(std::move(wait));
  if (set_barrier) {
    flushBarriers();
  }
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine