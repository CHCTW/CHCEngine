#pragma once
#include "../ClassName.h"

#include <wrl/client.h>

#include <string>
#include <unordered_map>
#include "../DescriptorHeap.h"

using Microsoft::WRL::ComPtr;
namespace CHCEngine {
namespace Renderer {
namespace Context {
struct ContextCommand;
class GraphicsContext;
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
  RESOURCE_TYPE_SAMPLER,
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
class Resource {
  friend class Renderer;
  friend struct CHCEngine::Renderer::Context::ContextCommand;
  friend class CHCEngine::Renderer::Pipeline::BindLayout;
  friend class ResourcePool; 

protected:
  ComPtr<GPUResource> gpu_resource_;
  ResourceInformation information_;
  // used by dynamic resource, all static resource will use dynmaic upload
  // buffer to update
  ComPtr<GPUResource> upload_buffer_;
  void *upload_buffer_map_pointer_ = nullptr;
  std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
      descriptor_ranges_;
  std::vector<std::pair<DescriptorType, unsigned int>> usage_indices_;
  ComPtr<GPUResource> getGPUResource() { return gpu_resource_; }
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
      std::unordered_map<DescriptorType, std::shared_ptr<DescriptorRange>>
          &descriptor_ranges);
  CPUDescriptorHandle getRTVHandleByUsageIndex(unsigned int index);
  CPUDescriptorHandle getDSVHandleByUsageIndex(unsigned int index);
  GPUDescriptorHandle getCBVSRVUAVUsagebyIndex(unsigned int index);
public:
  Resource &operator=(Resource &ref) = delete;
  Resource() = delete;
  void setName(std::string_view name);
  const std::string &getName() const { return information_.name_; }
  ResourceType getType() { return information_.type_; }
  ResourceInformation getInformation() { return information_; }
};
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine