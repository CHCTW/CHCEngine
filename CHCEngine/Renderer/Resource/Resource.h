#pragma once
#include "../ClassName.h"

#include <wrl/client.h>

#include "../DescriptorHeap.h"
#include <string>
#include <unordered_map>

using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
namespace Context {
struct ContextCommand;
class GraphicsContext;
class CopyContext;
class ComputeContext;
struct TrackingState;
class ContextResourceState;
} // namespace Context
namespace Pipeline {
class BindLayout;
}
namespace Resource {
class ResourcePool;
enum class ResourceType {
  RESOURCE_TYPE_BUFFER,
  RESOURCE_TYPE_TEXTURE,
  RESOURCE_TYPE_SWAP_CHAIN,
  RESOURCE_TYPE_GROUP,
};
enum class ResourceUpdateType {
  RESOURCE_UPDATE_TYPE_NONE,
  RESOURCE_UPDATE_TYPE_STATIC,
  RESOURCE_UPDATE_TYPE_DYNAMIC,
};
struct ResourceInformation {
  std::string name_;
  ResourceType type_;
  ResourceUpdateType update_type_;
};
struct ResourceDescriptorRange {
  std::shared_ptr<DescriptorRange> copy_usage_descriptors_ = nullptr;
  std::shared_ptr<DescriptorRange> bind_usage_descriptors_ = nullptr;
};

class Resource {
  friend class Renderer;
  friend struct CHCEngine::Renderer::Context::ContextCommand;
  friend class CHCEngine::Renderer::Pipeline::BindLayout;
  friend class ResourcePool;
  friend class Context::CopyContext;
  friend class ResourceGroup;
  friend struct Context::TrackingState;
  friend class Context::ContextResourceState;

protected:
  ComPtr<GPUResource> gpu_resource_;
  ResourceInformation information_;
  // used by dynamic resource, all static resource will use dynmaic upload
  // buffer to update
  ComPtr<GPUResource> upload_buffer_;
  void *upload_buffer_map_pointer_ = nullptr;
  ResourceDescriptorRange resource_descriptor_range_;
  std::vector<SubResourceState> sub_resource_states_;
  const ComPtr<GPUResource> &getGPUResource() { return gpu_resource_; }
  Resource(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
           const std::string &name, ResourceType type,
           ResourceUpdateType usage);
  Resource(ComPtr<GPUResource> gpu_resource, const std::string &name,
           ResourceType type, ResourceUpdateType usage)
      : gpu_resource_(gpu_resource), information_{name, type, usage} {}
  Resource(ComPtr<GPUResource> gpu_resource, ResourceInformation information)
      : gpu_resource_(gpu_resource), information_{information} {}
  Resource(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
           ResourceInformation information);
  Resource(ComPtr<GPUResource> gpu_resource, ComPtr<GPUResource> upload_buffer,
           ResourceInformation information,
           ResourceDescriptorRange &resource_desc_range);
  GPUDescriptorHandle getCBVSRVUAVUsagebyIndex(unsigned int index);
  CPUDescriptorHandle getCPUCBVSRVUAVUsagebyIndex(unsigned int index);
  std::vector<SubResourceState> &getSubResrouceStates();
  bool isSubResroucesSameStates();

public:
  virtual bool isAutoDecay() { return false; };
  Resource &operator=(Resource &ref) = delete;
  Resource() = delete;
  void setName(std::string_view name);
  const std::string &getName() const { return information_.name_; }
  ResourceType getType() { return information_.type_; }
  ResourceInformation getInformation() { return information_; }
  // 0 for group, 1 for buffer and swapchain
  unsigned int getSubResrouceCount() const;
};
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine