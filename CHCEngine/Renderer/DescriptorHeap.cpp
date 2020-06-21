#include "ClassName.h"

#include "DescriptorHeap.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "D3D12Convert.h"
#include "D3D12Utilities.hpp"
#include "Resource/SwapChainBuffer.h"
namespace CHCEngine {
namespace Renderer {
DescriptorHeap::DescriptorHeap(ComPtr<Device> device, DescriptorType type,
                               unsigned int size, const std::string &name,
                               bool shader_visible)
    : type_(type), size_(size), descriptor_size_(0), device_(device),
      name_(name), id_counter_(0), shader_visible_(false) {
  if (size == 0) {
    throw std::runtime_error(name_ + " Can't set 0 size descriptor heap");
  }
  // only srv_cbv_uav and sampler needs gpu side
  D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  if (shader_visible && (type == DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV ||
                         type == DescriptorType::DESCRIPTOR_TYPE_SAMPLER)) {
    flag = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    shader_visible_ = true;
  }
  D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc = {
      convertToD3DDescriptorHeapType(type), size, flag};
  ThrowIfFailed(device->CreateDescriptorHeap(&desc_heap_desc,
                                             IID_PPV_ARGS(&descriptors_)));
  NAME_D3D12_OBJECT_STRING(descriptors_, name);
  descriptor_size_ = device->GetDescriptorHandleIncrementSize(
      convertToD3DDescriptorHeapType(type));
  unused_range_[0] = size;
  range_size_table_.emplace(std::make_pair(size, 0), unused_range_.begin());
  start_index_table_.emplace(std::make_pair(0, size), unused_range_.begin());
}

std::shared_ptr<DescriptorRange>
DescriptorHeap::allocateRange(unsigned int size) {
  std::lock_guard<std::mutex> lock(descritpr_mutex_);
  // range_size_table_[std::make_pair(0, 1)] = unused_range_.end();
  auto find = range_size_table_.lower_bound(std::make_pair(size, 0));
  if (find == range_size_table_.end()) {
    throw std::exception((name_ + " Out of descriptor heap space").c_str());
  }
  unsigned int start = find->second->first;
  unsigned int length = find->second->second;
  range_size_table_.erase(std::make_pair(length, start));
  start_index_table_.erase(std::make_pair(start, length));
  unused_range_.erase(start);
  if (length != size) {
    unsigned int new_start = start + size;
    unsigned int new_length = length - size;
    unused_range_[new_start] = new_length;
    auto iter = unused_range_.find(new_start);
    range_size_table_.emplace(std::make_pair(new_length, new_start), iter);
    start_index_table_.emplace(std::make_pair(new_start, new_length), iter);
  }
  CPUDescriptorHandle handle =
      descriptors_->GetCPUDescriptorHandleForHeapStart();
  handle.offset(start, descriptor_size_);
  GPUDescriptorHandle gpu_handle;
  if (shader_visible_) {
    gpu_handle = descriptors_->GetGPUDescriptorHandleForHeapStart();
    gpu_handle.offset(start, descriptor_size_);
  }
  struct tempDescriptorRange : public DescriptorRange {
    tempDescriptorRange(unsigned long long id, unsigned int start_index,
                        unsigned int size, CPUDescriptorHandle first_handle,
                        unsigned int descriptor_size,
                        std::weak_ptr<DescriptorHeap> range_owner,
                        bool auto_free)
        : DescriptorRange(id, start_index, size, first_handle, descriptor_size,
                          range_owner, auto_free) {}
  };

  struct tempSahderVizDescriptorRange : public DescriptorRange {
    tempSahderVizDescriptorRange(unsigned long long id,
                                 unsigned int start_index, unsigned int size,
                                 CPUDescriptorHandle first_handle,
                                 GPUDescriptorHandle first_gpu_handle,
                                 unsigned int descriptor_size,
                                 std::weak_ptr<DescriptorHeap> range_owner,
                                 bool auto_free)
        : DescriptorRange(id, start_index, size, first_handle, first_gpu_handle,
                          descriptor_size, range_owner, auto_free) {}
  };

  std::shared_ptr<DescriptorRange> ret;
  if (!shader_visible_) {
    ret = std::make_shared<tempDescriptorRange>(id_counter_, start, size,
                                                handle, descriptor_size_,
                                                weak_from_this(), true);
  } else {
    ret = std::make_shared<tempSahderVizDescriptorRange>(
        id_counter_, start, size, handle, gpu_handle, descriptor_size_,
        weak_from_this(),
        true);
  }

  DescriptorRange range(id_counter_, start, size, handle, descriptor_size_,
                        weak_from_this());
  if (shader_visible_)
    range.shader_visible = true;
  ++id_counter_;
  allocalted_range_.emplace(range.id_, range);
  return ret; // namespace*/
              // Renderer
}
void DescriptorHeap::freeRange(const unsigned long long id) {
  std::lock_guard<std::mutex> lock(descritpr_mutex_);
  if (allocalted_range_.find(id) != allocalted_range_.end()) {
    auto find = allocalted_range_.find(id);
    unsigned int start = find->second.start_index_;
    unsigned int length = find->second.size_;
    unsigned int next_index = start + length;
    if (!start_index_table_.empty()) {
      bool combine_back = false;
      bool combine_front = false;
      auto back_unused =
          start_index_table_.lower_bound(std::make_pair(next_index, 0));
      auto front_unused = start_index_table_.end();
      if (back_unused != start_index_table_.end()) {
        if (back_unused != start_index_table_.begin()) {
          front_unused = back_unused;
          --front_unused;
        }
        if (next_index == back_unused->second->first) {
          combine_back = true;
        }
      } else { // didn't find back, check the last one
        --front_unused;
      }
      if (front_unused != start_index_table_.end() &&
          front_unused->second->first + front_unused->second->second == start) {
        combine_front = true;
      }
      if (combine_front) { // update index and length
        // std::cout << "front free : " << front_unused->second->first << ","
        //          << front_unused->second->second << std::endl;
        start = front_unused->second->first;
        length += front_unused->second->second;
        range_size_table_.erase(std::make_pair(front_unused->second->second,
                                               front_unused->second->first));
        unused_range_.erase(front_unused->second->first);
        start_index_table_.erase(front_unused);
      }
      if (combine_back) { // update length
        // std::cout << "back free : " << back_unused->second->first << ","
        //          << back_unused->second->second << std::endl;
        length += back_unused->second->second;
        range_size_table_.erase(std::make_pair(back_unused->second->second,
                                               back_unused->second->first));
        unused_range_.erase(back_unused->second->first);
        start_index_table_.erase(back_unused);
      }
    }
    allocalted_range_.erase(id);
    unused_range_.emplace(start, length);
    auto iter = unused_range_.find(start);
    range_size_table_.emplace(std::make_pair(length, start), iter);
    start_index_table_.emplace(std::make_pair(start, length), iter);
  } else {
    std::cout << "already free this id :" << id << std::endl;
  }
}
DescriptorRange::~DescriptorRange() {
  // std::cout << "descruct" << std::endl;
  if (auto_free_) {
    if (auto owner = range_owner_.lock())
      owner->freeRange(id_);
    else
      std::cout << "already free descriptor heap" << std::endl;
  }
}
} // namespace Renderer
} // namespace CHCEngine