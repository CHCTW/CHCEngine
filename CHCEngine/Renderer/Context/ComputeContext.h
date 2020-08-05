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
}
namespace Context {
class ComputeContext : public CopyContext {
 protected:
  friend class Renderer;
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
  void bindComputeResource(const std::shared_ptr<Resource::Resource> &resource,
                            unsigned int usage_index, unsigned int slot_index,
                            BindType bind_type, bool direct_bind);
  void bindComputeResource(const std::shared_ptr<Resource::Resource> &resource,
                            unsigned int slot_index,
                            unsigned int usage_index = 0);
  void bindComputeResource(const std::shared_ptr<Resource::Resource> &resource,
                            const std::string &slot_name,
                            unsigned int usage_index = 0);
  void setStaticUsageHeap();
  void dispatch(unsigned int x, unsigned int y, unsigned int z);
};
}  // namespace Context
}  // namespace Renderer
}  // namespace CHCEngine