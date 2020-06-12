#include "Command.h"
#include "../ClassName.h"

#include "../D3D12Convert.h"
#include "../D3D12Utilities.hpp"
#include "../Resource/Buffer.h"
#include "../Resource/Resource.h"
#include "ContextPool.h"
#include "../Pipeline/Pipeline.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void ContextCommand::free() {
  referenced_resources_.clear();
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
}
void ContextCommand::updateBufferRegion(
    std::shared_ptr<Resource::Buffer> buffer, void *const data,
    unsigned long long data_byte_size, unsigned long long offset) {
  // current only for dynamic data, will have dynamic updload data for stataic
  // data later
  if (buffer->getInformation().usage_ ==
      Resource::ResourceUsage::RESOURCE_USAGE_DYNAMIC) {
    memcpy(buffer->upload_buffer_map_pointer_, data, data_byte_size);
    list_->CopyBufferRegion(buffer->gpu_resource_.Get(), 0,
                            buffer->upload_buffer_.Get(), offset,
                            data_byte_size);
  } else {
    throw std::exception(
        "Doesn't support static data upload yet, will add it really soon");
  }
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
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine