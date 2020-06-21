#include "../ClassName.h"

#include "../Resource/Buffer.h"
#include "../Resource/DynamicBuffer.h"
#include "Context.h"
#include "ContextPool.h"
#include "CopyContext.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void CopyContext::updateBuffer(std::shared_ptr<Resource::Buffer> buffer,
                               void const  *data,
                               unsigned long long data_byte_size,
                               unsigned long long offset) {
  if (!buffer) {
    throw std::exception("Can't update buffer with null buffer pointer");
  }
  if (!data) {
    throw std::exception("Can't update buffer with null pointer");
  }
  if (data_byte_size + offset > buffer->getBufferInformation().size_) {
    throw std::exception("Update data + offset is out of buffer range");
  }
  if (buffer->getInformation().update_type_ ==
      Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC) {
    context_command_->updateBufferRegion(buffer, data, data_byte_size, offset);
  } else {
    if (auto pool = pool_.lock()) {
      auto space = pool->getDynamicUploadBuffer()->reqeustSpace(data_byte_size);
      context_command_->updateBufferRegion(buffer, data, data_byte_size, offset,
                                           space);
    } else {
      throw std::exception(
          "Context Pool already deleted, can't get dynamic upload buffer");
    }
  }
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine