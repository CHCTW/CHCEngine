#include "../ClassName.h"
#include "ContextPool.h"

#include "../D3D12Convert.h"
#include "../D3D12Utilities.hpp"
#include "../Resource/DynamicBuffer.h"

using CHCEngine::Renderer::convertToD3D12CommandType;
namespace CHCEngine {
namespace Renderer {
namespace Context {
static unsigned int allocator_begin_size = 10;
static unsigned int list_begin_size = 10;
template class ContextPool<GraphicsContext>;
template class ContextPool<ComputeContext>;
template class ContextPool<CopyContext>;
std::shared_ptr<ContextCommand> ContextPoolBase::getContextCommand() {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  if (available_list_.empty()) {
    auto d3d12type = convertToD3D12CommandType(type_);
    ComPtr<CommandAllocator> allocator;
    ComPtr<CommandList> list;
    ThrowIfFailed(
        device_->CreateCommandAllocator(d3d12type, IID_PPV_ARGS(&allocator)));
    ThrowIfFailed(device_->CreateCommandList(0, d3d12type, allocator.Get(),
                                             nullptr, IID_PPV_ARGS(&list)));
    unsigned long long size = pool_.size();
    pool_.emplace_back(std::make_shared<ContextCommand>(size, allocator, list,
                                                        weak_from_this()));
    available_list_.emplace_back(size);
    std::cout << "pool size " << pool_.size() << std::endl;
  }
  auto available_index = available_list_.front();
  available_list_.pop_front();

  return pool_[available_index];
}
void ContextPoolBase::freeContextCommand(unsigned long long id) {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  available_list_.push_back(id);
}
ContextPoolBase::ContextPoolBase(
    ComPtr<Device> device, CommandType type,
    std::shared_ptr<DescriptorHeap> cbv_uav_srv_heap,
    std::shared_ptr<DescriptorHeap> sampler_heap)
    : device_(std::move(device)), type_(type), cbv_uav_srv_heap_(std::move(cbv_uav_srv_heap)),
      sampler_heap_(std::move(sampler_heap)) {
  auto d3d12type = convertToD3D12CommandType(type_);
  dynamic_upload_buffer_ = std::make_shared<Resource::DynamicBuffer>(device_);
 /* std::lock_guard<std::mutex> lock(pool_mutex_);
  for (unsigned int i = 0; i < allocator_begin_size; ++i) {
    ComPtr<CommandAllocator> allocator;
    ComPtr<CommandList> list;
    ThrowIfFailed(
        device_->CreateCommandAllocator(d3d12type, IID_PPV_ARGS(&allocator)));
    ThrowIfFailed(device_->CreateCommandList(0, d3d12type, allocator.Get(),
                                             nullptr, IID_PPV_ARGS(&list)));
    pool_.emplace_back(
        std::make_shared<ContextCommand>(i, allocator, list, weak_from_this()));
    available_list_.emplace_back(i);
  }*/
}
template <class ContextClass>
std::shared_ptr<ContextClass> ContextPool<ContextClass>::getContext(
    std::function<void(ContextClass *)> callback, bool async) {
  auto context = std::make_shared<ContextClass>(type_, getContextCommand(),
                                                weak_from_this());
  if (callback) {
    context->recordCommands(callback, async);
  }
  return context;
}
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine
