#pragma once
#include <wrl/client.h>

#include <list>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>

#include "../ClassName.h"
#include "Command.h"
#include "GraphicsContext.h"

using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Resource
{
class DynamicBuffer;
}
namespace Context {
class ContextPoolBase : public std::enable_shared_from_this<ContextPoolBase> {
 protected:
  CommandType type_;
  ComPtr<Device> device_;
  std::vector<std::shared_ptr<ContextCommand>> pool_;
  std::list<unsigned long long> available_list_;
  std::mutex pool_mutex_;
  std::shared_ptr<Resource::DynamicBuffer> dynamic_upload_buffer_;
 public:
  std::shared_ptr<ContextCommand> getContextCommand();
  void freeContextCommand(unsigned long long id);
  ContextPoolBase(ComPtr<Device> device, CommandType type);
  std::shared_ptr<Resource::DynamicBuffer> getDynamicUploadBuffer() {
    return dynamic_upload_buffer_;
  }
};
template <class ContextClass>
class ContextPool : public ContextPoolBase{
  friend ContextClass;
 public:
  ContextPool(ComPtr<Device> device, CommandType type)
      : ContextPoolBase(device,type) {}
  std::shared_ptr<ContextClass> getContext(
      std::function<void (ContextClass *)> callback,bool async = true);
};

}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine
