#include "Command.h"
#include "../ClassName.h"

#include "../D3D12Convert.h"
#include "../D3D12Utilities.hpp"
#include "../Pipeline/Pipeline.h"
#include "../Resource/Buffer.h"
#include "../Resource/DynamicBuffer.h"
#include "../Resource/Resource.h"
#include "../Resource/ResourceGroup.h"
#include "../Resource/Texture.h"
#include "../Sampler/Sampler.h"
#include "ContextPool.h"
#include "ContextResourceState.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void ContextCommand::free() {
  allocated_spaces_.clear();
  referenced_resources_.clear();
  referenced_samplers_.clear();
  pipeline_state_ = nullptr;
  graphics_bind_signature_ = nullptr;
  compute_bind_signature_ = nullptr;
  if (auto use_owner = owner_.lock()) {
    reset();
    use_owner->freeContextCommand(id_);
  }
  context_resource_states_.clear();
}

void ContextCommand::reset() {
  ThrowIfFailed(allocator_->Reset());
  ThrowIfFailed(list_->Reset(allocator_.Get(), nullptr));
}

void ContextCommand::close() { ThrowIfFailed(list_->Close()); }

void ContextCommand::resrourceTransition(std::vector<Transition> &transitions,
                                         std::vector<UAVWait> &waits) {
  std::vector<D3D12_RESOURCE_BARRIER> barriers_(transitions.size() +
                                                waits.size());
  for (int i = 0; i < transitions.size(); ++i) {
    barriers_[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers_[i].Flags =
        convertToD3D12ResourceBarrierFlags(transitions[i].flag_);
    barriers_[i].Transition.pResource =
        transitions[i].resource_->getGPUResource().Get();
    barriers_[i].Transition.StateBefore =
        convertToD3D12ResourceStates(transitions[i].before_state_);
    barriers_[i].Transition.StateAfter =
        convertToD3D12ResourceStates(transitions[i].after_state_);
    barriers_[i].Transition.Subresource = transitions[i].subresource_index_;
    /*std::shared_ptr<Resource::Resource> temp =
        std::move(transitions[i].resource_);*/
    // referenced_resources_.emplace_back(std::move(transitions[i].resource_));
  }
  unsigned int offset = static_cast<unsigned int>(transitions.size());
  for (unsigned int i = 0; i < waits.size(); ++i) {
    barriers_[static_cast<size_t>(offset) + i].Type =
        D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barriers_[static_cast<size_t>(offset) + i].UAV.pResource =
        waits[i].resource_->getGPUResource().Get();
    referenced_resources_.emplace_back(std::move(waits[i].resource_));
  }
  list_->ResourceBarrier(static_cast<unsigned int>(barriers_.size()),
                         barriers_.data());
}
void ContextCommand::clearSwapChainBuffer(CPUDescriptorHandle handle,
                                          const float *color) {
  list_->ClearRenderTargetView(handle, color, 0, nullptr);
}
void ContextCommand::setPipelineState(ComPtr<PipelineState> pipeline_state) {
  list_->SetPipelineState(pipeline_state.Get());
  pipeline_state_ = std::move(pipeline_state);
}
void ContextCommand::updateBufferRegion(
    std::shared_ptr<Resource::Buffer> buffer, void const *data,
    unsigned long long data_byte_size, unsigned long long offset) {
  memcpy(buffer->upload_buffer_map_pointer_, data, data_byte_size);
  list_->CopyBufferRegion(buffer->gpu_resource_.Get(), offset,
                          buffer->upload_buffer_.Get(), 0, data_byte_size);
  // referenced_resources_.emplace_back(std::move(buffer));
}
void ContextCommand::updateBufferRegion(
    std::shared_ptr<Resource::Buffer> buffer, void const *data,
    unsigned long long data_byte_size, unsigned long long offset,
    std::shared_ptr<Resource::AllocateSpace> &allocate_space) {

  memcpy(allocate_space->copy_point_, data, data_byte_size);
  list_->CopyBufferRegion(buffer->gpu_resource_.Get(), offset,
                          allocate_space->buffer_.Get(),
                          allocate_space->gpu_offset_, data_byte_size);
  allocated_spaces_.emplace_back(std::move(allocate_space));
}
void ContextCommand::drawInstanced(unsigned int vertex_count,
                                   unsigned int instance_count,
                                   unsigned int start_vertex_location,
                                   unsigned int start_instance_location) {
  list_->DrawInstanced(vertex_count, instance_count, start_vertex_location,
                       start_instance_location);
}
void ContextCommand::setViewport(const Pipeline::Viewport &viewport) {
  list_->RSSetViewports(1, &viewport);
}
void ContextCommand::setScissor(const Pipeline::Scissor &scissor) {
  list_->RSSetScissorRects(1, &scissor);
}
void ContextCommand::setVertexBuffers(
    const std::vector<std::shared_ptr<Resource::Buffer>> &buffers) {
  D3D12_VERTEX_BUFFER_VIEW vbs[D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
  for (unsigned int i = 0; i < buffers.size(); ++i) {
    vbs[i] = *buffers[i]->vertex_buffer_view_;
    // referenced_resources_.emplace_back(buffers[i]);
  }
  list_->IASetVertexBuffers(0, static_cast<unsigned int>(buffers.size()), vbs);
}
void ContextCommand::setTopology(PrimitiveTopology topology) {
  list_->IASetPrimitiveTopology(convertToD3DPrimitiveTopology(topology));
}
void ContextCommand::setRenderTarget(CPUDescriptorHandle handle) {
  list_->OMSetRenderTargets(1, &handle, false, nullptr);
}
void ContextCommand::setGraphicsBindSignature(
    ComPtr<BindSignature> bind_signature) {
  list_->SetGraphicsRootSignature(bind_signature.Get());
  graphics_bind_signature_ = std::move(bind_signature);
}
void ContextCommand::setComputeBindSignature(
    ComPtr<BindSignature> bind_signature) {
  list_->SetComputeRootSignature(bind_signature.Get());
  compute_bind_signature_ = std::move(bind_signature);
}
void ContextCommand::bindGraphicsResource(
    std::shared_ptr<Resource::Resource> resource, unsigned int usage_index,
    unsigned int slot_index, BindType bind_type, bool direct_bind) {
  if (!direct_bind) {
    list_->SetGraphicsRootDescriptorTable(
        slot_index, resource->getCBVSRVUAVUsagebyIndex(usage_index));
  } else {
    // special case when bin group resource to a direct bind
    if (resource->getInformation().type_ ==
        Resource::ResourceType::RESOURCE_TYPE_GROUP) {
      resource =
          std::move(std::static_pointer_cast<Resource::ResourceGroup>(resource)
                        ->getResource(usage_index));
    }
    switch (getUsage(bind_type)) {
    case BindUsage::BIND_USAGE_CBV:
      list_->SetGraphicsRootConstantBufferView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    case BindUsage::BIND_USAGE_SRV:
      list_->SetGraphicsRootShaderResourceView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    case BindUsage::BIND_USAGE_UAV:
      list_->SetGraphicsRootUnorderedAccessView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    }
  }
  referenced_resources_.emplace_back(std::move(resource));
}
void ContextCommand::bindGraphicsResource(const Resource::Resource *resource,
                                          unsigned int usage_index,
                                          unsigned int slot_index,
                                          BindType bind_type,
                                          bool direct_bind) {

  if (!direct_bind) {
    list_->SetGraphicsRootDescriptorTable(
        slot_index, resource->getCBVSRVUAVUsagebyIndex(usage_index));
  } else {
    // special case when bin group resource to a direct bind
    if (resource->getInformation().type_ ==
        Resource::ResourceType::RESOURCE_TYPE_GROUP) {
      const auto &p =
          static_cast<const Resource::ResourceGroup *>(resource)->getResource(
              usage_index);
      resource = p.get();
    }
    switch (getUsage(bind_type)) {
    case BindUsage::BIND_USAGE_CBV:
      list_->SetGraphicsRootConstantBufferView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    case BindUsage::BIND_USAGE_SRV:
      list_->SetGraphicsRootShaderResourceView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    case BindUsage::BIND_USAGE_UAV:
      list_->SetGraphicsRootUnorderedAccessView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    }
  }
}
void ContextCommand::bindComputeResource(
    std::shared_ptr<Resource::Resource> resource, unsigned int usage_index,
    unsigned int slot_index, BindType bind_type, bool direct_bind) {
  if (!direct_bind) {
    list_->SetComputeRootDescriptorTable(
        slot_index, resource->getCBVSRVUAVUsagebyIndex(usage_index));
  } else {
    // special case when bin group resource to a direct bind
    if (resource->getInformation().type_ ==
        Resource::ResourceType::RESOURCE_TYPE_GROUP) {
      resource =
          std::move(std::static_pointer_cast<Resource::ResourceGroup>(resource)
                        ->getResource(usage_index));
    }
    switch (getUsage(bind_type)) {
    case BindUsage::BIND_USAGE_CBV:
      list_->SetComputeRootConstantBufferView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    case BindUsage::BIND_USAGE_SRV:
      list_->SetComputeRootShaderResourceView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    case BindUsage::BIND_USAGE_UAV:
      list_->SetComputeRootUnorderedAccessView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    }
  }
  referenced_resources_.emplace_back(std::move(resource));
}
void ContextCommand::bindComputeResource(const Resource::Resource *resource,
                                         unsigned int usage_index,
                                         unsigned int slot_index,
                                         BindType bind_type, bool direct_bind) {
  if (!direct_bind) {
    list_->SetComputeRootDescriptorTable(
        slot_index, resource->getCBVSRVUAVUsagebyIndex(usage_index));
  } else {
    // special case when bin group resource to a direct bind
    if (resource->getInformation().type_ ==
        Resource::ResourceType::RESOURCE_TYPE_GROUP) {
      const auto &p =
          static_cast<const Resource::ResourceGroup *>(resource)->getResource(
              usage_index);
      resource = p.get();
    }
    switch (getUsage(bind_type)) {
    case BindUsage::BIND_USAGE_CBV:
      list_->SetComputeRootConstantBufferView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    case BindUsage::BIND_USAGE_SRV:
      list_->SetComputeRootShaderResourceView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    case BindUsage::BIND_USAGE_UAV:
      list_->SetComputeRootUnorderedAccessView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
      break;
    }
  }
}
void ContextCommand::bindGraphicsSampler(
    std::shared_ptr<Sampler::Sampler> sampler, unsigned int usage_index,
    unsigned int slot_index) {
  list_->SetGraphicsRootDescriptorTable(slot_index,
                                        sampler->getSampleHandler(usage_index));
  referenced_samplers_.emplace_back(std::move(sampler));
}
void ContextCommand::setStaticDescriptorHeap() {
  auto pool = owner_.lock();
  if (!pool) {
    throw std::exception("Can't set static descriptorheap when context pool is "
                         "already released");
  }
  auto srv_cbv_uav = pool->getCBVUAVSRVHeap();
  auto sampler_heap = pool->getSamplerHeap();
  ID3D12DescriptorHeap *heaps[2];
  heaps[0] = srv_cbv_uav->descriptors_.Get();
  heaps[1] = sampler_heap->descriptors_.Get();
  list_->SetDescriptorHeaps(2, heaps);
}
void ContextCommand::updateTextureRegion(
    std::shared_ptr<Resource::Texture> texture,
    const std::vector<D3D12_TEXTURE_COPY_LOCATION> &src_layouts,
    const std::vector<D3D12_TEXTURE_COPY_LOCATION> &dst_layouts) {
  for (unsigned int i = 0; i < src_layouts.size(); ++i) {
    list_->CopyTextureRegion(&dst_layouts[i], 0, 0, 0, &src_layouts[i],
                             nullptr);
  }
  // referenced_resources_.emplace_back(texture);
}
void ContextCommand::updateTextureRegion(
    std::shared_ptr<Resource::Texture> texture,
    const std::vector<D3D12_TEXTURE_COPY_LOCATION> &src_layouts,
    const std::vector<D3D12_TEXTURE_COPY_LOCATION> &dst_layouts,
    std::vector<std::shared_ptr<Resource::AllocateSpace>> &spaces) {
  for (unsigned int i = 0; i < src_layouts.size(); ++i) {
    list_->CopyTextureRegion(&dst_layouts[i], 0, 0, 0, &src_layouts[i],
                             nullptr);
  }
  // referenced_resources_.emplace_back(texture);
  allocated_spaces_.insert(allocated_spaces_.end(),
                           std::make_move_iterator(spaces.begin()),
                           std::make_move_iterator(spaces.end()));
}
void ContextCommand::dispatch(unsigned int x, unsigned int y, unsigned int z) {
  list_->Dispatch(x, y, z);
}
void ContextCommand::referenceTrackingResrouce(
    std::unordered_map<std::shared_ptr<Resource::Resource>,
                       ContextResourceState> &context_resource_states) {
  context_resource_states_ = std::move(context_resource_states);
  /*for (auto &p : context_resource_states) {
    referenced_resources_.emplace_back(p.first);
  }*/
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine