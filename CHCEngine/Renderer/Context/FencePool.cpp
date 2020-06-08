#include "../ClassName.h"
#include "FencePool.h"

#include "../D3D12Utilities.hpp"
namespace CHCEngine {
namespace Renderer {
namespace Context {
static unsigned int fence_begin_size = 10;
FencePool::FencePool(ComPtr<Device> device) : device_(device) {
  /*for (unsigned int i = 0; i < fence_begin_size; ++i) {
    ComPtr<Fence> fence;
    ThrowIfFailed(
        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    fence_pool_.push_back(
        std::make_shared<BaseFence>(fence, i, weak_from_this()));
    available_list_.emplace_back(i);
  }*/
}
void FencePool::freeBaseFence(unsigned long long id) {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  available_list_.emplace_back(id);
}
std::shared_ptr<ContextFence> FencePool::getContextFence() {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  for (auto it = available_list_.begin(); it != available_list_.end(); ++it) {
    if (fence_pool_[*it]->getState() == FenceState::FENCE_STATE_IDLE) {
      auto index = *it;
      available_list_.erase(it);
      return std::make_shared<ContextFence>(fence_pool_[index]);
    }
  }
  ComPtr<Fence> fence;
  ThrowIfFailed(
      device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

  auto index = fence_pool_.size();
  fence_pool_.push_back(
      std::make_shared<BaseFence>(fence, index, weak_from_this()));
  available_list_.emplace_back(index);
  return std::make_shared<ContextFence>(fence_pool_[fence_pool_.size() - 1]);
  
}
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine