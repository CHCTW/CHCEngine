#pragma once
#include "Context.h"

#include <memory>

namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Resource {
class Buffer;
class DynamicBuffer;
class Texture;
} // namespace Resource
namespace Context {
class CopyContext : public Context {
protected:
  friend class Renderer;

public:
  virtual ~CopyContext() { /* waitRecordingDone();*/
  }
  CopyContext(CommandType type, std::shared_ptr<ContextCommand> context_command,
              std::weak_ptr<ContextPoolBase> pool)
      : Context(type, context_command, pool) {}
  void updateBuffer(const std::shared_ptr<Resource::Buffer> &buffer,
                    const void *data, unsigned long long data_byte_size,
                    unsigned long long offset = 0);
  // multi planar copy is not support yet, need some use case to have good define
  void updateTexture(
      const std::shared_ptr<Resource::Texture> &texture,
      const void *consecutive_image_data, unsigned int array_start_index = 0,
      unsigned int array_count = (std::numeric_limits<unsigned int>::max)(),
      unsigned int mip_start_level = 0,
      unsigned int mip_count = (std::numeric_limits<unsigned int>::max)());
};
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine