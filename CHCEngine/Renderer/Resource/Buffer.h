#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "Resource.h"

namespace CHCEngine {
namespace Renderer {
namespace Context {
struct ContextCommand;
} // namespace Context
class DescriptorRange;
namespace Resource {
enum class BufferType {
  BUFFER_TYPE_NONE = 0x0,
  BUFFER_TYPE_VERTEX = 0x1,
  BUFFER_TYPE_INDEX = 0x2,
  BUFFER_TYPE_CONSTANT = 0x4,
  BUFFER_TYPE_STRUCTERED = 0x8,
  BUFFER_TYPE_COUNTER = 0x10,
  BUFFER_TYPE_RAW = 0x20,
  BUFFER_TYPE_CUSTOM = 0xff,
};
// basiclly for struct buffer,
enum class BufferReadWrite {
  BUFFER_READ_WRITE_NONE = 0,
  BUFFER_READ_WRITE_READ = 1,
  BUFFER_READ_WRITE_READ_WRITE = 2,
};
using Attributes =
    std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>;
struct BufferInformation {
  BufferType type_ = BufferType::BUFFER_TYPE_NONE;
  BufferReadWrite usage_ = BufferReadWrite::BUFFER_READ_WRITE_NONE;
  unsigned long long size_ = 0;
  unsigned int strdie_size_ = 0;
  // use sematic name as key to get the offset and format
  // the sematic name should always be the same as in the hlsl shader
  Attributes vetex_attributes_;
};
// won't have any creation here, all creation is done in the resource pool,
// according to what it needs it will generate the proper needs field;
// goint to have more different type here, vertex,index,counter.....
class Buffer : public Resource {
protected:
  friend struct Context::ContextCommand;
  BufferInformation buffer_information_;
  // for different view possible; like counter buffer, will have read and write
  std::shared_ptr<VertexBufferView> vertex_buffer_view_;
  std::shared_ptr<IndexBufferView> index_buffer_view_;

public:
  Buffer &operator=(Buffer &ref) = delete;
  // Vertex Buffer Initialize
  Buffer(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
         ResourceInformation information, BufferInformation buffer_information,
         std::shared_ptr<VertexBufferView> vertex_buffer_view_);
  // index Buffer initialize
  Buffer(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
         ResourceInformation information, BufferInformation buffer_information,
         std::shared_ptr<IndexBufferView> index_buffer_view);

  Buffer(
      ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
      ResourceInformation information, BufferInformation buffer_information,
      const std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
          &descriptor_ranges,
      const std::vector<std::pair<DescriptorType, unsigned int>>& descriptor_indices,
      std::shared_ptr<VertexBufferView> vertex_buffer_view,
      std::shared_ptr<IndexBufferView> index_buffer_view);

  const BufferInformation &getBufferInformation();
  BufferType getBufferType();
};
} // namespace Resource

} // namespace Renderer
} // namespace CHCEngine