#pragma once
#include "../ClassName.h"

#include <memory>
#include <mutex>
#include <queue>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Resource {
// an internal use buffer, help for uploading data to buffer and texture, also
// this can be help on constant buffer, for binding that use pure desc, recycle
// the memeory whe fence complete, so it's a thread safe structure

// thought : the request should be fast, the release won't have to release
// immediately
// alocation(start,size) , and store a current( start, end)
// when allocate(size), end will plus
// when free we first check whether the (start,size) fit with the current
// start, yes, we free and check the free table.
// no we store the free in free table
// this give us fast fetch, we assume the space should be return really soon
// so even some times there will be some space that already return
// we won't just really free the space we will free the space until the
// space before it has been return, some key thourght : will the start always
// be differnt in the table??
struct RingBuffer;
class DynamicBuffer;
// use for internal, use should never use this
struct AllocateSpace {
  // used for return
  unsigned long long start_ = 0;
  unsigned long long size_ = 0;
  // used for update
  ComPtr<GPUResource> buffer_;
  unsigned long long gpu_offset_;
  void *copy_point_;
  std::weak_ptr<RingBuffer> owner_;
  std::weak_ptr<DynamicBuffer> dynamic_buffer_;
  ~AllocateSpace();
};
struct RingBuffer : public std::enable_shared_from_this<RingBuffer> {
  unsigned long long size_ = 0;
  unsigned long long remain_space_ = 0;
  unsigned long long start_ = 0;
  unsigned long long end_ = 0;
  unsigned long long alignment_ = 0;
  ComPtr<GPUResource> buffer_;
  void *cpu_map_buffer_ = nullptr;
  std::weak_ptr<DynamicBuffer> owner_;
  std::unordered_map<unsigned long long, unsigned long long> free_table_;
  std::mutex buffer_mutex_;
  RingBuffer(unsigned long long size, ComPtr<GPUResource> buffer_,
             void *map_point, std::weak_ptr<DynamicBuffer> owner,
             unsigned long long alignment = 0);
  // return true and fill the space table if have enough space
  bool RequestSpace(unsigned long long request_size,
                    std::shared_ptr<AllocateSpace> &space);
  // used by the allocate space, return true if the space is totaly free
  bool FreeSpace(unsigned long long start, unsigned long long size);
};
class DynamicBuffer : public std::enable_shared_from_this<DynamicBuffer> {
private:
  friend struct AllocateSpace;
  ComPtr<Device> device_;
  unsigned long long alignment_ = 0;
  unsigned long long default_size_ = 0;
  std::shared_ptr<RingBuffer> createRingBuffer(unsigned long long size);
  std::queue<std::shared_ptr<RingBuffer>> ring_buffers_;
  std::mutex dynamic_buffer_mutex_;
  void freeUnusedRingBuffer();

public:
  DynamicBuffer(ComPtr<Device> device_, unsigned long long alignment_ = 1,
                unsigned long long default_size = 2048);
  std::shared_ptr<AllocateSpace> requestSpace(unsigned long long size);
};
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine