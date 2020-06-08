#pragma once

#include <list>
#include <memory>
#include <mutex>

#include "../ClassName.h"
#include "Fence.h"

using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
namespace Context {
class FencePool : public std::enable_shared_from_this<FencePool> {
 private:
  ComPtr<Device> device_;
  std::vector<std::shared_ptr<BaseFence>> fence_pool_;
  // when get availavle this will check the state of fence first
  // if the state is idle, we use fence
  std::list<unsigned long long> available_list_;
  std::mutex pool_mutex_;
 public:
  FencePool(ComPtr<Device> device);
  void freeBaseFence(unsigned long long id);
  std::shared_ptr<ContextFence> getContextFence();
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine