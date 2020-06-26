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
      vertex_buffer_view_(vertex_buffer_view_) {}
Buffer::Buffer(ComPtr<GPUResource> gpu_resource,
               ComPtr<GPUResource> upload_buffer,
               ResourceInformation information,
               BufferInformation buffer_information,
               std::shared_ptr<IndexBufferView> index_buffer_view)
    : Resource(gpu_resource, upload_buffer, information),
      buffer_information_(buffer_information),
      index_buffer_view_(index_buffer_view) {}
Buffer::Buffer(
    ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
    ResourceInformation information, BufferInformation buffer_information,
    std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
        &descriptor_ranges,
    std::shared_ptr<VertexBufferView> vertex_buffer_view,
    std::shared_ptr<IndexBufferView> index_buffer_view)
    : Resource(gpu_resource, upload_buffer, information, descriptor_ranges),
      buffer_information_(buffer_information),
      vertex_buffer_view_(vertex_buffer_view),
      index_buffer_view_(index_buffer_view) {}
const BufferInformation &Buffer::getBufferInformation() {
  return buffer_information_;
}
BufferType Buffer::getBufferType() { return buffer_information_.type_; }
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine