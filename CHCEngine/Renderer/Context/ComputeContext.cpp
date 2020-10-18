#include "../ClassName.h"

#include "../Pipeline/Pipeline.h"
#include "../Resource/ResourceGroup.h"
#include "../Resource/Texture.h"
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
  while (compute_bind_descriptors_.size())
    compute_bind_descriptors_.pop();
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
    uint32_t format_index, uint32_t usage_index,
    const std::shared_ptr<Pipeline::BindLayout> &layout) {
  switch (resource->getType()) {
  case Resource::ResourceType::RESOURCE_TYPE_BUFFER:
    updateBindingBufferState(resource, slot_index, format_index, usage_index,
                             layout);
    break;
  case Resource::ResourceType::RESOURCE_TYPE_TEXTURE:
    updateBindingTextureState(resource, slot_index, format_index, usage_index,
                              layout);
    break;
  case Resource::ResourceType::RESOURCE_TYPE_GROUP:
    const auto &slot = layout->getBindSlot(slot_index);
    const Resource::ResourceGroup *group =
        static_cast<Resource::ResourceGroup *>(resource.get());
    updateContextResourceState(resource, ResourceState::RESOURCE_STATE_UNKNOWN,
                               false, 0);
    uint32_t group_index = 0;
    for (uint32_t f = 0; f < slot.formats_.size(); ++f) {
      for (uint32_t i = 0; i < slot.formats_[f].resource_count_; ++i) {
        const auto &res = group->getResource(group_index);
        uint32_t u_index = group->getUsageIndex(group_index);
        updateBindingResrouceState(res, slot_index, f, u_index, layout);
        ++group_index;
      }
    }
    break;
  }
}

void ComputeContext::updateBindingBufferState(
    const std::shared_ptr<Resource::Resource> &resource, uint32_t slot_index,
    uint32_t format_index, uint32_t usage_index,
    const std::shared_ptr<Pipeline::BindLayout> &layout) {
  BindType type = layout->getBindSlot(slot_index).formats_[format_index].type_;
  ShaderType visibility = layout->getBindSlot(slot_index).visibility_;
  // const auto &buffer = std::static_pointer_cast<Resource::Buffer>(resource);
  /*if (!checkBindTypeUsageType(type,
                              buffer->getBufferUsage(usage_index).usage_)) {
                              //D3D12 will issue warning here, maybe don't need
  to check

  }*/
  auto state = getStateFromBindTypeVisibility(type, visibility);
  updateContextResourceState(resource, state, false, 0);
}

void ComputeContext::updateBindingTextureState(
    const std::shared_ptr<Resource::Resource> &resource, uint32_t slot_index,
    uint32_t format_index, uint32_t usage_index,
    const std::shared_ptr<Pipeline::BindLayout> &layout) {
  BindType type = layout->getBindSlot(slot_index).formats_[format_index].type_;
  ShaderType visibility = layout->getBindSlot(slot_index).visibility_;
  Resource::Texture *texture = static_cast<Resource::Texture *>(resource.get());
  auto state = getStateFromBindTypeVisibility(type, visibility);
  const auto &sub_resource_list =
      texture->getUsageSubResourceIndices(usage_index);
  for (auto i : sub_resource_list) {
    updateContextResourceState(resource, state, false, i);
  }
}

void ComputeContext::flushComputeBindings() {
  while (compute_bind_descriptors_.size()) {
    auto &bind = compute_bind_descriptors_.front();
    context_command_->bindComputeResource(bind.resource_, bind.usage_index_,
                                          bind.slot_index_, bind.bind_type_,
                                          bind.direct_bind_);
    compute_bind_descriptors_.pop();
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
  updateBindingResrouceState(resource, slot_index, 0, usage_index,
                             compute_layout_);
  compute_bind_descriptors_.push(
      {resource.get(), usage_index, slot_index, type, direct_bind});
  /*bindComputeResource(resource, usage_index, slot_index, type, direct_bind);*/
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
void ComputeContext::bindComputeConstants(const void *data,
                                          uint32_t num_32bit_constant,
                                          const std::string &slot_name,
                                          uint32_t constant_offset) {
  if (!compute_layout_) {
    throw std::exception(
        "Need to set compute bind layout first in this context");
  }
  unsigned int slot_index = compute_layout_->getSlotIndex(slot_name);
  bindComputeConstants(data, num_32bit_constant, slot_index, constant_offset);
}
void ComputeContext::bindComputeConstants(const void *data,
                                          uint32_t num_32bit_constant,
                                          uint32_t slot_index,
                                          uint32_t constant_offset) {
  if (!compute_layout_) {
    throw std::exception(
        "Need to set compute bind layout first in this context");
  }
  auto total =
      compute_layout_->getBindSlot(slot_index).formats_[0].resource_count_;
  if (num_32bit_constant + constant_offset > total) {
    throw std::exception("Bind Compute Constants bind too many constants or "
                         "have too large offset");
  }
  context_command_->bindComputeConstants(data, slot_index, num_32bit_constant,
                                         constant_offset);
}
void ComputeContext::setStaticUsageHeap() {
  context_command_->setStaticDescriptorHeap();
}
void ComputeContext::dispatch(unsigned int x, unsigned int y, unsigned int z) {
  flushBarriers();
  flushComputeBindings();
  context_command_->dispatch(x, y, z);
}
void ComputeContext::resourceTransition(
    const std::shared_ptr<Resource::Resource> &resource,
    ResourceState before_state, ResourceState after_state, bool set_barrier,
    ResourceTransitionFlag flag, unsigned int subresource_index) {
  transitions_.push_back(
      {resource.get(), before_state, after_state, flag, subresource_index});
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