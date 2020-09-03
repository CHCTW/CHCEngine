#pragma once
#include "CopyContext.h"

namespace CHCEngine {
namespace Renderer {
class Renderer;
namespace Resource {
class DynamicBuffer;
} // namespace Resource
namespace Pipeline {
class Pipeline;
struct BindSlot;
}
namespace Context {
class ComputeContext : public CopyContext {
 protected:
  friend class Renderer;
   void bindComputeResource(const std::shared_ptr<Resource::Resource> &resource,
                            unsigned int usage_index, unsigned int slot_index,
                            BindType bind_type, bool direct_bind);
 public:
  virtual ~ComputeContext() { 
     // waitRecordingDone(); 
  }
  ComputeContext(CommandType type,
                 std::shared_ptr<ContextCommand> context_command,
              std::weak_ptr<ContextPoolBase> pool)
      : CopyContext(type, context_command, pool) {}
  void setPipeline(const std::shared_ptr<Pipeline::Pipeline> &pipline);
  void setComputeBindLayout(
      const std::shared_ptr<Pipeline::BindLayout> &bind_layout);
  /*void bindComputeResource(const std::shared_ptr<Resource::Resource> &resource,
                           unsigned int usage_index, unsigned int slot_index,const Pipeline::BindSlot& slot);*/
  void bindComputeResource(const std::shared_ptr<Resource::Resource> &resource,
                            unsigned int slot_index,
                            unsigned int usage_index = 0);
  void bindComputeResource(const std::shared_ptr<Resource::Resource> &resource,
                            const std::string &slot_name,
                            unsigned int usage_index = 0);
  void setStaticUsageHeap();
  void dispatch(unsigned int x, unsigned int y, unsigned int z);
  void
  resourceTransition(const std::shared_ptr<Resource::Resource> &resource,
                     ResourceState before_state, ResourceState after_state,
                     bool set_barrier = false,
                     ResourceTransitionFlag flag =
                         ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_NONE,
                     unsigned int subresource_index = all_subresrouce_index);
  void uavResourceWait(const std::shared_ptr<Resource::Resource> & resource,
                       bool set_barrier = false);
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine