#pragma once
#include <wrl/client.h>

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "ClassName.h"
using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
namespace Context {
struct ContextCommand;
}
// descritpor table is not goint to dispose for user,
// there is going to be a stataic descriptor heap to
// store all descriptors but not going to use for binding
//  there is going to be another descriptor heap for
// binding, all PSO will request certain nubmers of continous
//  descriptor and use  it to bind, will use descriptor copy
// all descirptor heap area should have good memeory management

// every descriptor heap should have ablility to check
// the all available memeory block, should be a link list
class SwapChainBuffer;
class DescriptorHeap;

class DescriptorRange {
private:
  friend class DescriptorHeap;
  unsigned long long id_;
  unsigned int start_index_;
  unsigned int size_;
  CPUDescriptorHandle first_handle_;
  GPUDescriptorHandle first_gpu_handle_;
  unsigned int descriptor_size_;
  unsigned int local_index_ = 0;
  CPUDescriptorHandle current_handle_;
  std::weak_ptr<DescriptorHeap> range_owner_;
  bool shader_visible = false;
  bool auto_free_ = false;
  DescriptorRange(unsigned long long id, unsigned int start_index,
                  unsigned int size, CPUDescriptorHandle first_handle,
                  unsigned int descriptor_size,
                  std::weak_ptr<DescriptorHeap> range_owner,
                  bool auto_free = false)
      : id_(id), start_index_(start_index), size_(size),
        first_handle_(first_handle), descriptor_size_(descriptor_size),
        current_handle_(first_handle_), range_owner_(range_owner),
        auto_free_(auto_free) {}
  DescriptorRange(unsigned long long id, unsigned int start_index,
                  unsigned int size, CPUDescriptorHandle first_handle,
                  GPUDescriptorHandle first_gpu_handle,
                  unsigned int descriptor_size,
                  std::weak_ptr<DescriptorHeap> range_owner,
                  bool auto_free = false)
      : DescriptorRange(id, start_index, size, first_handle, descriptor_size,
                        range_owner, false) {
    first_gpu_handle_ = first_gpu_handle;
    shader_visible = true;
  }

public:
  DescriptorRange() = delete;
  ~DescriptorRange();

  CPUDescriptorHandle getNextAvailableHandle() {
    if (local_index_ == size_) {
      return CPUDescriptorHandle();
    }
    CPUDescriptorHandle ret = current_handle_;
    current_handle_.offset(1, descriptor_size_);
    ++local_index_;
    return ret;
  }
  CPUDescriptorHandle getHandle(unsigned int index) {
    if (index >= size_) {
      return CPUDescriptorHandle();
    }
    CPUDescriptorHandle ret = first_handle_;
    ret.offset(index, descriptor_size_);
    return ret;
  }
  GPUDescriptorHandle getGPUHandle(unsigned int index) {
    if (!shader_visible)
      return GPUDescriptorHandle();
    GPUDescriptorHandle ret = first_gpu_handle_;
    ret.offset(index, descriptor_size_);
    return ret;
  }
  unsigned int getSize() { return size_;
  }
  unsigned long long getId() { return id_; }
};
// a thread safe class, probaly going to store all of them in Renender.

class DescriptorHeap : public std::enable_shared_from_this<DescriptorHeap> {
  friend struct Context::ContextCommand;
private:
  std::string name_;
  unsigned long long id_counter_;
  // recorded used range, need id when needs to free it
  std::unordered_map<unsigned long long, DescriptorRange> allocalted_range_;
  // used to save free range use the startIndes as key
  std::map<unsigned int, unsigned int> unused_range_;
  // use to fast fech to available size range, (size,start)
  std::map<std::pair<unsigned int, unsigned int>,
           const std::map<unsigned int, unsigned int>::iterator>
      range_size_table_;
  // use to fast free range to combine some range in the unused_range_
  // (start,size)
  std::map<std::pair<unsigned int, unsigned int>,
           const std::map<unsigned int, unsigned int>::iterator>
      start_index_table_;
  std::unordered_map<std::string, unsigned int> descriptor_index_;
  DescriptorType type_;
  unsigned int size_;
  unsigned int descriptor_size_;
  ComPtr<Descriptors> descriptors_;
  ComPtr<Device> device_;
  bool shader_visible_;
  // since there might be a condition that all fall into cycle call for free
  // just add a note, although afer adding auto_free_, I think is fine
  std::mutex descritpr_mutex_;

public:
  DescriptorHeap() = delete;
  DescriptorHeap(ComPtr<Device> device, DescriptorType type, unsigned int size,
                 const std::string &name, bool shader_visible);
  std::shared_ptr<DescriptorRange> allocateRange(unsigned int size);
  void freeRange(const unsigned long long id);
};
} // namespace Renderer
} // namespace CHCEngine