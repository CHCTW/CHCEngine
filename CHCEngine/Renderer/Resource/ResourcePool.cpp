#include "../ClassName.h"
#include "ResourcePool.h"

#include <magic_enum.hpp>

#include "../D3D12Convert.h"
#include "Utility.hpp"

namespace CHCEngine {
namespace Renderer {
namespace Resource {
unsigned long long ResourcePool::getNextBufferId(BufferType type) {
  std::lock_guard<std::mutex> lock(pool_mutex_);
  if (!buffer_id_count_.count(type)) buffer_id_count_[type] = 0;
  return buffer_id_count_[type]++;
}
ResourcePool::ResourcePool(ComPtr<Device> device,
                           std::shared_ptr<DescriptorHeap> srv_uav_cbv,
                           std::shared_ptr<DescriptorHeap> rtv,
                           std::shared_ptr<DescriptorHeap> dsv)
    : device_(device) {
  static_heaps_[DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV] = srv_uav_cbv;
  static_heaps_[DescriptorType::DESCRIPTOR_TYPE_RTV] = rtv;
  static_heaps_[DescriptorType::DESCRIPTOR_TYPE_DSV] = dsv;
}

std::shared_ptr<Buffer> ResourcePool::getVertexBuffer(
    unsigned int vertex_count,
    const std::vector<std::pair<std::string, DataFormat>>& attributes,
    ResourceState initial_state, ResourceUsage usage) {
  unsigned int stride_size = 0;
  unsigned int byte_size = 0;
  std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>
      vetex_attributes;
  for (auto& p : attributes) {
    byte_size = getDataFormatByteSize(p.second);
    if (byte_size == 0) {
      throw std::exception((std::string(magic_enum::enum_name(p.second)) +
                            "  is not a valid format for vertex attributes")
                               .c_str());
    }
    if (vetex_attributes.count(p.first)) {
      throw std::exception(
          (p.first + "have the same sematic name in one vertex attributes")
              .c_str());
    }
    vetex_attributes[p.first] = std::make_pair(stride_size, p.second);
    stride_size += byte_size;
  }
  unsigned buffer_size = stride_size * vertex_count;
  auto id = getNextBufferId(BufferType::BUFFER_TYPE_VERTEX);
  std::string name = "vertex_buffer_" + std::to_string(id);
  auto gpu_resource = createBuffer(device_, buffer_size,
                                   HeapType::HEAP_TYPE_DEFAULT, initial_state);
  NAME_D3D12_OBJECT_STRING(gpu_resource, name);
  ComPtr<GPUResource> upload_buffer;
  if (usage == ResourceUsage::RESOURCE_USAGE_DYNAMIC) {
    upload_buffer =
        createBuffer(device_, buffer_size, HeapType::HEAP_TYPE_UPLOAD,
                     ResourceState::RESOURCE_STATE_COPY_SOURCE);
    std::string temp = name;
    temp += "_upload";
    NAME_D3D12_OBJECT_STRING(upload_buffer, temp);
  }
  ResourceInformation res_info = {name, ResourceType::RESOURCE_TYPE_BUFFER,
                                  usage};
  BufferInformation buffer_information = {
      BufferType::BUFFER_TYPE_VERTEX, BufferUsage::BUFFER_USAGE_READ,
                                          buffer_size,
                                          stride_size,
                                          vetex_attributes};
  auto vertex_view = std::make_shared<VertexBufferView>();

  vertex_view->BufferLocation = gpu_resource->GetGPUVirtualAddress();
  vertex_view->SizeInBytes = buffer_size;
  vertex_view->StrideInBytes = stride_size;

  return std::make_shared<Buffer>(gpu_resource, upload_buffer, res_info,
                                  buffer_information, vertex_view);

  /*
    struct tempSwapChain : public Resource::SwapChainBuffer {
  tempSwapChain(ComPtr<GPUResource> buf,
                std::shared_ptr<DescriptorRange> range, std::string name,
                int x, int y)
      : SwapChainBuffer(buf, range, name, x, y) {}
};
  */
}

std::shared_ptr<Buffer> ResourcePool::getIndexBuffer(
    unsigned int index_count, IndexFormat index_format,
    ResourceState initial_state, ResourceUsage usage) {
  auto id = getNextBufferId(BufferType::BUFFER_TYPE_INDEX);
  std::string name = "index_buffer_" + std::to_string(id);


  std::unordered_map<std::string, std::pair<unsigned int, DataFormat>>
      vetex_attributes;
  DataFormat format = DataFormat::DATA_FORMAT_R32_UINT;
  unsigned index_size = 4;
  if (index_format == IndexFormat::INDEX_FORMAT_16_UINT) {
    format = DataFormat::DATA_FORMAT_R16_UINT;
    index_size = 2;
  }
  vetex_attributes["INDEX"] = std::make_pair(0, format);
  unsigned int buffer_size = 4 * index_count;
  auto gpu_resource = createBuffer(device_, buffer_size,
                                   HeapType::HEAP_TYPE_DEFAULT, initial_state);
  NAME_D3D12_OBJECT_STRING(gpu_resource, name);
  ComPtr<GPUResource> upload_buffer;
  if (usage == ResourceUsage::RESOURCE_USAGE_DYNAMIC) {
    upload_buffer =
        createBuffer(device_, buffer_size, HeapType::HEAP_TYPE_UPLOAD,
                     ResourceState::RESOURCE_STATE_COPY_SOURCE);
    std::string temp = name;
    temp += "_upload";
    NAME_D3D12_OBJECT_STRING(upload_buffer, temp);
  }
  ResourceInformation res_info = {name, ResourceType::RESOURCE_TYPE_BUFFER,
                                  usage};
  BufferInformation buffer_information = {
      BufferType::BUFFER_TYPE_INDEX, BufferUsage::BUFFER_USAGE_READ,
      buffer_size, index_size,vetex_attributes};
  auto index_view = std::make_shared<IndexBufferView>();

  index_view->BufferLocation = gpu_resource->GetGPUVirtualAddress();
  index_view->SizeInBytes = buffer_size;
  index_view->Format = convertToDXGIFormat(index_format);

  return std::make_shared<Buffer>(gpu_resource, upload_buffer, res_info,
                                  buffer_information, index_view);
}

}  // namespace Resource
}  // namespace Renderer
}  // namespace CHCEngine