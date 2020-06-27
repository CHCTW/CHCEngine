#include "../ClassName.h"

#include "../Resource/Buffer.h"
#include "../Resource/DynamicBuffer.h"
#include "../Resource/Texture.h"
#include "Context.h"
#include "ContextPool.h"
#include "CopyContext.h"


namespace CHCEngine {
namespace Renderer {

namespace Context {
void CopyContext::updateBuffer(const std::shared_ptr<Resource::Buffer> &buffer,
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
void CopyContext::updateTexture(
    const std::shared_ptr<Resource::Texture> &texture,
    const void *consecutive_image_data, unsigned int array_start_index,
    unsigned int array_count, unsigned int mip_start_level,
    unsigned int mip_count) {
  auto inf = texture->getTextureInformation();
  if (array_start_index>=inf.depth_) {
    throw std::exception((std::string("Can't update texture with array start index : ")+std::to_string(array_start_index)+" the texture depth is : "+std::to_string(inf.depth_)).c_str());
  }
  if (mip_start_level >= inf.depth_) {
    throw std::exception(
        (std::string("Can't update texture with mip start level : ") +
         std::to_string(mip_start_level) +
         " the texture depth is : " + std::to_string(inf.mip_levels_))
            .c_str());
  }
  array_count = (std::min)(array_count, inf.depth_ - array_start_index);
  mip_count = (std::min)(mip_count, inf.mip_levels_ - mip_start_level);
  for (unsigned int i = array_start_index; i < array_start_index+array_count ; ++i) {
    for (unsigned int j = mip_start_level ; j < mip_start_level+mip_count ; ++j) {


    }
  }

}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine