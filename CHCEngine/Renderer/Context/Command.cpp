#include "Command.h"
#include "../ClassName.h"

#include "../D3D12Convert.h"
#include "../D3D12Utilities.hpp"
#include "../Pipeline/Pipeline.h"
#include "../Resource/Buffer.h"
#include "../Resource/DynamicBuffer.h"
#include "../Resource/Resource.h"
#include "ContextPool.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void ContextCommand::free() {
  allocated_spaces_.clear();
  referenced_resources_.clear();
  pipeline_state_ = nullptr;
  graphics_bind_signature_ = nullptr;
  if (auto use_owner = owner_.lock()) {
    reset();
    use_owner->freeContextCommand(id_);
  }
}

void ContextCommand::reset() {
  ThrowIfFailed(allocator_->Reset());
  ThrowIfFailed(list_->Reset(allocator_.Get(), nullptr));
}

void ContextCommand::close() { ThrowIfFailed(list_->Close()); }

void ContextCommand::resrourceTransition(std::vector<Transition> &transitions) {
  std::vector<D3D12_RESOURCE_BARRIER> barriers_(transitions.size());
  for (int i = 0; i < transitions.size(); ++i) {
    referenced_resources_.emplace_back(transitions[i].resource);
    barriers_[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers_[i].Flags =
        convertToD3D12ResourceBarrierFlags(transitions[i].flag);
    barriers_[i].Transition.pResource =
        transitions[i].resource->getGPUResource().Get();
    barriers_[i].Transition.StateBefore =
        convertToD3D12ResourceStates(transitions[i].before_state);
    barriers_[i].Transition.StateAfter =
        convertToD3D12ResourceStates(transitions[i].after_state);
    barriers_[i].Transition.Subresource = transitions[i].subresource_index;
  }
  list_->ResourceBarrier(static_cast<unsigned int>(transitions.size()),
                         barriers_.data());
}
void ContextCommand::clearSwapChainBuffer(CPUDescriptorHandle handle,
                                          const float *color) {
  list_->ClearRenderTargetView(handle, color, 0, nullptr);
}
void ContextCommand::setPipelineState(ComPtr<PipelineState> pipeline_state) {
  list_->SetPipelineState(pipeline_state.Get());
  pipeline_state_ = pipeline_state;
}
void ContextCommand::updateBufferRegion(
    std::shared_ptr<Resource::Buffer> buffer, void const * data,
    unsigned long long data_byte_size, unsigned long long offset) {
  // current only for dynamic data, will have dynamic updload data for stataic
  // data later
  memcpy(buffer->upload_buffer_map_pointer_, data, data_byte_size);
  list_->CopyBufferRegion(buffer->gpu_resource_.Get(), offset,
                          buffer->upload_buffer_.Get(), 0, data_byte_size);
}
void ContextCommand::updateBufferRegion(
    std::shared_ptr<Resource::Buffer> buffer, void const * data,
    unsigned long long data_byte_size, unsigned long long offset,
    std::shared_ptr<Resource::AllocateSpace> allocate_space) {

  memcpy(allocate_space->copy_point_, data, data_byte_size);
  list_->CopyBufferRegion(buffer->gpu_resource_.Get(), offset,
                          allocate_space->buffer_.Get(),
                          allocate_space->gpu_offset_, data_byte_size);
  allocated_spaces_.push_back(allocate_space);
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
  for (unsigned int i = 0 ; i < buffers.size() ; ++i) {
    referenced_resources_.emplace_back(buffers[i]);
    vbs[i] = *buffers[i]->vertex_buffer_view_;
  }
  list_->IASetVertexBuffers(0,static_cast<unsigned int>(buffers.size()), vbs);
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
  graphics_bind_signature_ = bind_signature;
}
void ContextCommand::bindGraphcisResource(
    std::shared_ptr<Resource::Resource> resource, 
    unsigned int usage_index, unsigned int slot_index,
    BindType bind_type,
    bool direct_bind) {
  referenced_resources_.push_back(resource);
  if (!direct_bind) {
    list_->SetGraphicsRootDescriptorTable(
        slot_index, resource->getGPUHandleByUsageIndex(usage_index));
  }else {
      switch (getUsage(bind_type)) {
    case BindUsage::BIND_USAGE_CBV:
        list_->SetGraphicsRootConstantBufferView(
            slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
    case BindUsage::BIND_USAGE_SRV:
      list_->SetGraphicsRootShaderResourceView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
    case BindUsage::BIND_USAGE_UAV:
      list_->SetGraphicsRootUnorderedAccessView(
          slot_index, resource->gpu_resource_->GetGPUVirtualAddress());
    }
  }
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine