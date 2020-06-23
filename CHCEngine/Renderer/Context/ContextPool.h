#pragma once
#include <wrl/client.h>

#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "../ClassName.h"
#include "Command.h"
#include "GraphicsContext.h"

using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
class Renderer;
class DescriptorHeap;
namespace Resource {
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
  std::shared_ptr<DescriptorHeap> cbv_uav_srv_heap_;
  std::shared_ptr<DescriptorHeap> sampler_heap_;

public:
  std::shared_ptr<ContextCommand> getContextCommand();
  void freeContextCommand(unsigned long long id);
  ContextPoolBase(ComPtr<Device> device, CommandType type,
                  std::shared_ptr<DescriptorHeap> cbv_uav_srv_heap,
                  std::shared_ptr<DescriptorHeap> sampler_heap);
  std::shared_ptr<Resource::DynamicBuffer> getDynamicUploadBuffer() {
    return dynamic_upload_buffer_;
  }
  std::shared_ptr<DescriptorHeap> &getCBVUAVSRVHeap() {
    return cbv_uav_srv_heap_;
  }
  std::shared_ptr<DescriptorHeap> &getSamplerHeap() { return sampler_heap_; }
};
template <class ContextClass> class ContextPool : public ContextPoolBase {
  friend ContextClass;

public:
  ContextPool(ComPtr<Device> device, CommandType type,
              std::shared_ptr<DescriptorHeap> cbv_uav_srv_heap,
              std::shared_ptr<DescriptorHeap> sampler_heap)
      : ContextPoolBase(device, type, cbv_uav_srv_heap, sampler_heap) {}
  std::shared_ptr<ContextClass>
  getContext(std::function<void(ContextClass *)> callback, bool async = true);
};

} // namespace Context
} // namespace Renderer
} // namespace CHCEngine
