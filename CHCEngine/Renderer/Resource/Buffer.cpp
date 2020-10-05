#include "../ClassName.h"

#include "Buffer.h"
namespace CHCEngine {
namespace Renderer {
namespace Resource {
Buffer::Buffer(ComPtr<GPUResource> gpu_resource,
               ComPtr<GPUResource> upload_buffer,
               ResourceInformation information,
               BufferInformation buffer_information,
               std::shared_ptr<VertexBufferView> vertex_buffer_view_)
    : Resource(gpu_resource, upload_buffer, information),
      buffer_information_(buffer_information),
      vertex_buffer_view_(vertex_buffer_view_) {
  sub_resource_states_.push_back({ResourceState::RESOURCE_STATE_COMMON,
                                  ResourceState::RESOURCE_STATE_COMMON});
}
Buffer::Buffer(ComPtr<GPUResource> gpu_resource,
               ComPtr<GPUResource> upload_buffer,
               ResourceInformation information,
               BufferInformation buffer_information,
               std::shared_ptr<IndexBufferView> index_buffer_view)
    : Resource(gpu_resource, upload_buffer, information),
      buffer_information_(buffer_information),
      index_buffer_view_(index_buffer_view) {
  sub_resource_states_.push_back({ResourceState::RESOURCE_STATE_COMMON,
                                  ResourceState::RESOURCE_STATE_COMMON});
}
Buffer::Buffer(ComPtr<GPUResource> gpu_resource,
               ComPtr<GPUResource> upload_buffer,
               ResourceInformation information,
               BufferInformation buffer_information,
               ResourceDescriptorRange &resource_desc_range,
               std::shared_ptr<VertexBufferView> vertex_buffer_view,
               std::shared_ptr<IndexBufferView> index_buffer_view,
               const std::vector<BufferUsage> &usages)
    : Resource(gpu_resource, upload_buffer, information, resource_desc_range),
      buffer_information_(buffer_information),
      vertex_buffer_view_(vertex_buffer_view),
      index_buffer_view_(index_buffer_view), usages_(usages_) {
  sub_resource_states_.push_back({ResourceState::RESOURCE_STATE_COMMON,
                                  ResourceState::RESOURCE_STATE_COMMON});
}
const BufferInformation &Buffer::getBufferInformation() {
  return buffer_information_;
}
BufferType Buffer::getBufferType() { return buffer_information_.type_; }
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine