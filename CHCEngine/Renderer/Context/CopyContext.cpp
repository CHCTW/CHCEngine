#include "../ClassName.h"

#include "Context.h"
#include "CopyContext.h"
#include "../Resource/Buffer.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void CopyContext::updateBuffer(std::shared_ptr<Resource::Buffer> buffer,
                               void *const data,unsigned long long data_byte_size, unsigned long long offset) {
  if (!buffer) {
    throw std::exception("Can't update buffer with null buffer pointer");
  }
  if (!data) {
    throw std::exception("Can't update buffer with null pointer");
  }
  if (data_byte_size+offset>buffer->getBufferInformation().size_) {
    throw std::exception("Update data + offset is out of buffer range");
  }
  context_command_->updateBufferRegion(buffer, data, data_byte_size, offset);
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine