#include "../ClassName.h"

#include "DynamicBuffer.h"

#include "../D3D12Utilities.hpp"

#include <iostream>
namespace CHCEngine {
namespace Renderer {
namespace Resource {
AllocateSpace::~AllocateSpace() {
  if (auto use_owner = owner_.lock()) {
    if (use_owner->FreeSpace(start_, size_)) {
      if (auto dynmaic = dynamic_buffer_.lock()) {
        dynmaic->freeUnusedRingBuffer();
      }
    }
  }
}
RingBuffer::RingBuffer(unsigned long long size, ComPtr<GPUResource> buffer,
                       void *map_point,
                       std::weak_ptr<DynamicBuffer> owner,
                       unsigned long long alignment)
    : size_(size), remain_space_(size), buffer_(buffer),
      cpu_map_buffer_(map_point),
      alignment_(alignment), owner_(owner) {}
bool RingBuffer::RequestSpace(unsigned long long request_size,
                              std::shared_ptr<AllocateSpace> space) {
  std::lock_guard<std::mutex> lock(buffer_mutex_);
  request_size = (request_size + alignment_) & ~alignment_;
  /*std::cout << "request size : " << request_size
            << " with start :" << start_ <<
      " with end :" << end_ << "  with remain space :" << remain_space_<<std::endl;*/
  if (remain_space_ < request_size) {
    return false;
  }
  // the continue space is at the middle of the buffer
  if (end_ < start_) {
    if (remain_space_ < request_size) {
      return false;
    }
    space->buffer_ = buffer_;
    // char for one byte
    space->copy_point_ = (char *)cpu_map_buffer_ + end_;
    space->gpu_offset_ = end_;
    space->owner_ = weak_from_this();
    space->size_ = request_size;
    space->start_ = end_;
    space->dynamic_buffer_ = owner_;
    end_ += request_size;
    end_ %= size_;
    remain_space_ -= request_size;

    return true;
    // check front and back remain
  } else {
    if (size_ - end_ >= request_size) {
      // normal case ,just go for it
      space->buffer_ = buffer_;
      // char for one byte
      space->copy_point_ = (char *)cpu_map_buffer_ + end_;
      space->gpu_offset_ = end_;
      space->owner_ = weak_from_this();
      space->size_ = request_size;
      space->start_ = end_;
      space->dynamic_buffer_ = owner_;
      end_ += request_size;
      end_ %= size_;
      remain_space_ -= request_size;
      return true;
    }
    if (remain_space_ - (size_ - end_) >= request_size) {
      // normal case ,just go for it
      space->buffer_ = buffer_;
      // we start from the begining
      space->copy_point_ = cpu_map_buffer_;
      space->gpu_offset_ = 0;
      space->owner_ = weak_from_this();
      space->size_ = request_size + (size_ - end_);
      space->start_ = end_;
      end_ = request_size;
      end_ %= size_;
      space->dynamic_buffer_ = owner_;
      // can't use request_size + (size_ - end_) will cause over flow.. hmm
      remain_space_ -= (space->size_);
      return true;
    }
    return false;
  }
}
bool RingBuffer::FreeSpace(unsigned long long start, unsigned long long size) {
  std::lock_guard<std::mutex> lock(buffer_mutex_);
  if (start_ == start) {
    start_ += size;
    start_ %= size_;
    remain_space_ += size;
   /* std::cout << "free buffer with start :" << start << "  with end :" << end_
              << " with free size :"<<size
              << " remain space with " << remain_space_ << std::endl;*/
    // free if already in the table
    while (free_table_.count(start_)) {
      auto it = free_table_.find(start_);
    std::cout << start_ << "  " << it->second << std::endl;
      start_ += it->second;
      remain_space_ += it->second;
      free_table_.erase(it);
      start_ %= size_;
    }
  } else {
    free_table_[start] = size;
  }
  //std::cout << "Ring free end" << std::endl;
  if (remain_space_ == size_)
    return true;
  return false;
}
std::shared_ptr<RingBuffer>
DynamicBuffer::createRingBuffer(unsigned long long size) {

  size = (size + alignment_) & ~alignment_;
  ComPtr<GPUResource> buffer;
  ComPtr<GPUResource> res;
  D3D12_HEAP_PROPERTIES heap_property;
  heap_property.Type = D3D12_HEAP_TYPE_UPLOAD;
  heap_property.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_property.CreationNodeMask = 1;
  heap_property.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_property.VisibleNodeMask = 1;

  D3D12_RESOURCE_DESC desc;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Width = size;
  desc.Alignment = 0;
  desc.Flags = D3D12_RESOURCE_FLAG_NONE;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  device_->CreateCommittedResource(&heap_property, D3D12_HEAP_FLAG_NONE, &desc,
                                   D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                   IID_PPV_ARGS(&buffer));
  void *map_point = nullptr;

  D3D12_RANGE readRange{0, 0};
  ThrowIfFailed(buffer->Map(0, &readRange, &map_point));
  return std::make_shared<RingBuffer>(size, buffer, map_point,
                                      weak_from_this(),alignment_);
}
void DynamicBuffer::freeUnusedRingBuffer() {
  //std::cout << "free ring buffe with queue size : " << ring_buffers_.size()
    //        << std::endl;
  std::lock_guard<std::mutex> lock(dynamic_buffer_mutex_);
  while (ring_buffers_.size() > 1) {
    if (ring_buffers_.front()->size_ != ring_buffers_.front()->remain_space_)
      break;
    ring_buffers_.pop();
  }

}
DynamicBuffer::DynamicBuffer(ComPtr<Device> device,
                             unsigned long long alignment,
                             unsigned long long default_size)
    : device_(device), alignment_(alignment), default_size_(default_size) {}
std::shared_ptr<AllocateSpace>
DynamicBuffer::reqeustSpace(unsigned long long size) {
  auto allocate = std::make_shared<AllocateSpace>();
  std::lock_guard<std::mutex> lock(dynamic_buffer_mutex_);
  if (ring_buffers_.empty()) {
    ring_buffers_.push(
        createRingBuffer((std::max)(default_size_, default_size_)));
  }
  if (!ring_buffers_.back()->RequestSpace(size, allocate)) {
    //std::cout << "create new queue" << std::endl;
    ring_buffers_.push(
        createRingBuffer((std::max)(ring_buffers_.back()->size_, size) * 3));
    ring_buffers_.back()->RequestSpace(size, allocate);
  }
  return allocate;
}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine