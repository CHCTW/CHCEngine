#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "Resource.h"


namespace CHCEngine {
namespace Renderer {
class DescriptorRange;
namespace Resource {
enum class BufferType {
  BUFFER_TYPE_NONE,
  BUFFER_TYPE_VERTEX,
  BUFFER_TYPE_INDEX,
  BUFFER_TYPE_CONSTANT,
  BUFFER_TYPE_STRUCTERED,
  BUFFER_TYPE_COUNTER,
  BUFFER_TYPE_RAW,
  BUFFER_TYPE_CUSTOM
};
// basiclly for construct buffer, 
enum class BufferUsage {
  BUFFER_USAGE_NONE = 0,
  BUFFER_USAGE_READ = 1,
  BUFFER_USAGE_WRITE = 2,
};

struct BufferInformation {
  BufferType type_ = BufferType::BUFFER_TYPE_NONE;
  BufferUsage usage_ = BufferUsage::BUFFER_USAGE_NONE;
  unsigned int size_ = 0;
  unsigned int strdie_size_ = 0;
  // use sematic name as key to get the offset and format
  // the sematic name should always be the same as in the hlsl shader
  std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>
      vetex_attributes_;
};
// won't have any creation here, all creation is done in the resource pool,
// according to what it needs it will generate the proper needs field;
// goint to have more different type here, vertex,index,counter.....
class Buffer : public Resource {
 protected:
  BufferInformation buffer_information_;
  // for different view possible; like counter buffer, will have read and write
  std::unordered_map <
      DescriptorType,std::shared_ptr<DescriptorRange>> descriptors_;
  std::shared_ptr<VertexBufferView> vertex_buffer_view_;
  std::shared_ptr<IndexBufferView> index_buffer_view_;
 public:
  Buffer& operator=(Buffer& ref) = delete;
  // Vertex Buffer Initialize
  Buffer(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
         ResourceInformation information, BufferInformation buffer_information,
         std::shared_ptr<VertexBufferView> vertex_buffer_view_);
  Buffer(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
         ResourceInformation information, BufferInformation buffer_information,
         std::shared_ptr<IndexBufferView> index_buffer_view);
  const BufferInformation & getBufferInformation();
  BufferType getBufferType();
};
}  // namespace Resource

}  // namespace Renderer
}  // namespace CHCEngine