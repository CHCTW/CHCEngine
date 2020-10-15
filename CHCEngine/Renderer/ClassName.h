#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

// Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
inline void SetName(ID3D12Object *pObject, LPCWSTR name) {
  pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object *pObject, LPCWSTR name, UINT index) {
  WCHAR fullName[50];
  if (swprintf_s(fullName, L"%s[%u]", name, index) > 0) {
    pObject->SetName(fullName);
  }
}
inline std::wstring s2ws(const std::string &s) {
  int len;
  int slength = (int)s.length() + 1;
  len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
  wchar_t *buf = new wchar_t[len];
  MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
  std::wstring r(buf);
  delete[] buf;
  return r;
}
inline void SetNameString(ID3D12Object *pObject, std::string name) {
  std::wstring stemp = s2ws(name); // Temporary buffer is required
  LPCWSTR result = stemp.c_str();
  SetName(pObject, result);
}
#else
inline void SetName(ID3D12Object *, LPCWSTR) {}
inline void Set6NameIndexed(ID3D12Object *, LPCWSTR, UINT) {}
inline void SetNameString(ID3D12Object *pObject, std::string name) {}
#endif
namespace CHCEngine {
namespace Renderer {
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x);
#define NAME_D3D12_OBJECT_STRING(x, name)                                      \
  SetNameString((x).Get(), name.c_str());
using Device = ID3D12Device;
using Factory = IDXGIFactory4;
using SwapChain = IDXGISwapChain3;
// gpu and cpu handle is same now going to conisder combine them in future
struct GPUDescriptorHandle : public D3D12_GPU_DESCRIPTOR_HANDLE {
  GPUDescriptorHandle() { ptr = 0; } // namespace Renderer
  GPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
      : D3D12_GPU_DESCRIPTOR_HANDLE(handle) {}
  GPUDescriptorHandle(GPUDescriptorHandle &handle) { ptr = handle.ptr; }
  GPUDescriptorHandle &operator=(const GPUDescriptorHandle &other) {
    ptr = other.ptr;
    return *this;
  }
  void offset(unsigned int index, unsigned descriptor_size) {
    ptr += SIZE_T((INT64)index * (INT64)(descriptor_size));
  }
}; // namespace CHCEngine
struct CPUDescriptorHandle : public D3D12_CPU_DESCRIPTOR_HANDLE {
  CPUDescriptorHandle() { ptr = 0; }
  CPUDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle)
      : D3D12_CPU_DESCRIPTOR_HANDLE(handle) {}
  CPUDescriptorHandle(CPUDescriptorHandle &handle) { ptr = handle.ptr; }
  CPUDescriptorHandle &operator=(const CPUDescriptorHandle &other) {
    ptr = other.ptr;
    return *this;
  }
  void offset(unsigned int index, unsigned descriptor_size) {
    ptr += SIZE_T((INT64)index * (INT64)(descriptor_size));
  }
};
using Descriptors = ID3D12DescriptorHeap;
struct DescriptorHandle {
  GPUDescriptorHandle gpu_;
  CPUDescriptorHandle cpu_;
};
enum class DescriptorType {
  DESCRIPTOR_TYPE_SRV_UAV_CBV,
  DESCRIPTOR_TYPE_SAMPLER,
  DESCRIPTOR_TYPE_RTV,
  DESCRIPTOR_TYPE_DSV,
  DESCRIPTOR_TYPE_COUNT,
};

using CommandQueue = ID3D12CommandQueue;
using CommandAllocator = ID3D12CommandAllocator;
using CommandList = ID3D12GraphicsCommandList6;
using CommandListBase = ID3D12CommandList;
enum class CommandType {
  COMMAND_TYPE_GRAPHICS,
  COMMAND_TYPE_COMPUTE,
  COMMAND_TYPE_COPY,
  COMMAND_TYPE_QUEUE_BOUND,
  COMMAND_TYPE_BUNDLE,
  COMMAND_TYPE_COUNT,
};
enum class CommandAllocatorState {
  COMMAND_ALLOCATOR_STATE_RECORD,
  COMMAND_ALLOCATOR_STATE_EXECUTE,
  COMMAND_ALLOCATOR_STATE_COUNT
};
enum class CommandListState {
  COMMAND_LIST_STATE_RECORD,
  COMMAND_LIST_STATE_CLOSE,
  COMMAND_LIST_STATE_COUNT
};
using GPUResource = ID3D12Resource;
using Fence = ID3D12Fence;
enum class ResourceState : unsigned {
  RESOURCE_STATE_COMMON = 0,
  RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
  RESOURCE_STATE_INDEX_BUFFER = 0x2,
  RESOURCE_STATE_RENDER_TARGET = 0x4,
  RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
  RESOURCE_STATE_DEPTH_WRITE = 0x10,
  RESOURCE_STATE_DEPTH_READ = 0x20,
  RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
  RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
  RESOURCE_STATE_STREAM_OUT = 0x100,
  RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
  RESOURCE_STATE_COPY_DEST = 0x400,
  RESOURCE_STATE_COPY_SOURCE = 0x800,
  RESOURCE_STATE_RESOLVE_DEST = 0x1000,
  RESOURCE_STATE_RESOLVE_SOURCE = 0x2000,
  RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x400000,
  RESOURCE_STATE_SHADING_RATE_SOURCE = 0x1000000,
  RESOURCE_STATE_GENERIC_READ =
      (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
  RESOURCE_STATE_ALL_SHADER_RESOURCE = (0x40 | 0x80),
  RESOURCE_STATE_PRESENT = 0,
  RESOURCE_STATE_PREDICATION = 0x200,
  RESOURCE_STATE_VIDEO_DECODE_READ = 0x10000,
  RESOURCE_STATE_VIDEO_DECODE_WRITE = 0x20000,
  RESOURCE_STATE_VIDEO_PROCESS_READ = 0x40000,
  RESOURCE_STATE_VIDEO_PROCESS_WRITE = 0x80000,
  RESOURCE_STATE_VIDEO_ENCODE_READ = 0x200000,
  RESOURCE_STATE_VIDEO_ENCODE_WRITE = 0x800000,
  RESOURCE_STATE_UNKNOWN = 0xffffffff // only used for resrouce tracking
};
inline ResourceState operator|(const ResourceState &lhs,
                               const ResourceState &rhs) {
  return static_cast<ResourceState>(
      static_cast<std::underlying_type<ResourceState>::type>(lhs) |
      static_cast<std::underlying_type<ResourceState>::type>(rhs));
}
inline ResourceState operator&(const ResourceState &lhs,
                               const ResourceState &rhs) {
  return static_cast<ResourceState>(
      static_cast<std::underlying_type<ResourceState>::type>(lhs) &
      static_cast<std::underlying_type<ResourceState>::type>(rhs));
}

static unsigned int gen_read_ =
    static_cast<unsigned int>(ResourceState::RESOURCE_STATE_GENERIC_READ);
// will return unknown if it can't merge, only read state can merge
//  need to change , texture and buffer have different merge state
inline ResourceState mergeIfPossible(ResourceState left_state,
                                     ResourceState right_state) {
  ResourceState merge = ResourceState::RESOURCE_STATE_UNKNOWN;
  unsigned int left = static_cast<unsigned int>(left_state);
  unsigned int right = static_cast<unsigned int>(right_state);
  if ((left & gen_read_) && (right & gen_read_))
    return left_state | right_state;
  return merge;
}
inline bool isReadState(const ResourceState &state) {
  if (state == ResourceState::RESOURCE_STATE_UNKNOWN)
    return false;
  unsigned int left = static_cast<unsigned int>(state);
  unsigned int right = static_cast<unsigned int>(gen_read_);
  return (left & right) > 0;
}
inline bool needChange(const ResourceState &next,
                       const ResourceState &current) {
  // unknown always need to change
  if (current == ResourceState::RESOURCE_STATE_UNKNOWN)
    return true;
  unsigned int left = static_cast<unsigned int>(next);
  unsigned int right = static_cast<unsigned int>(current);
  // basiclly describe it is a substate of current state
  // actually one equal and reader state will get false
  return !((left & right) == right);
}
enum class ResourceTransitionFlag {
  RESOURCE_TRANSITION_FLAG_NONE = 0,
  RESOURCE_TRANSITION_FLAG_BEGIN = 0x1,
  RESOURCE_TRANSITION_FLAG_END = 0x2,
  RESOURCE_TRANSITION_FLAG_COUNT = 0x3
};
static const unsigned int resrouce_transition_flag_count_ =
    static_cast<unsigned int>(
        ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_COUNT);
static unsigned int all_subresrouce_index =
    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
namespace Resource {
class Resource;
};
struct Color {
  float r;
  float g;
  float b;
  float a;
};
using VertexBufferView = D3D12_VERTEX_BUFFER_VIEW;
using IndexBufferView = D3D12_INDEX_BUFFER_VIEW;

enum class HeapType {
  HEAP_TYPE_DEFAULT,
  HEAP_TYPE_UPLOAD,
  HEAP_TYPE_READBACK,
  HEAP_TYPE_CUSTOM
};
enum class IndexFormat {
  INDEX_FORMAT_NONE,
  INDEX_FORMAT_32_UINT,
  INDEX_FORMAT_16_UINT,
};

using Blob = ID3DBlob;
using ShaderReflection = ID3D12ShaderReflection;
// use byte in this order
// 1,2,4,8,16 for fast fetch size per elemnt
// only get the enum before DXGI_FORMAT_BC1_TYPELESS
// will add more in future
enum class DataFormat {
  DATA_FORMAT_UNKNOWN,
  DATA_FORMAT_R8_TYPELESS,
  DATA_FORMAT_R8_UNORM,
  DATA_FORMAT_R8_UINT,
  DATA_FORMAT_R8_SNORM,
  DATA_FORMAT_R8_SINT,
  DATA_FORMAT_A8_UNORM,
  DATA_FORMAT_ONE_BYTE_LAST, // R8
  DATA_FORMAT_R16_TYPELESS,
  DATA_FORMAT_R16_FLOAT,
  DATA_FORMAT_D16_UNORM,
  DATA_FORMAT_R16_UNORM,
  DATA_FORMAT_R16_UINT,
  DATA_FORMAT_R16_SNORM,
  DATA_FORMAT_R16_SINT,
  DATA_FORMAT_TWO_BYTE_ONE_CHANNEL_LAST, // R16
  DATA_FORMAT_R8G8_TYPELESS,
  DATA_FORMAT_R8G8_UNORM,
  DATA_FORMAT_R8G8_UINT,
  DATA_FORMAT_R8G8_SNORM,
  DATA_FORMAT_R8G8_SINT,
  DATA_FORMAT_TWO_BYTE_TWO_CHANNEL_LAST, // R8G8
  DATA_FORMAT_TWO_BYTE_LAST,
  DATA_FORMAT_R32_TYPELESS,
  DATA_FORMAT_D32_FLOAT,
  DATA_FORMAT_R32_FLOAT,
  DATA_FORMAT_R32_UINT,
  DATA_FORMAT_R32_SINT,
  DATA_FORMAT_FOUR_BYTE_ONE_CHANNEL_LAST, // R32
  DATA_FORMAT_R16G16_TYPELESS,
  DATA_FORMAT_R16G16_FLOAT,
  DATA_FORMAT_R16G16_UNORM,
  DATA_FORMAT_R16G16_UINT,
  DATA_FORMAT_R16G16_SNORM,
  DATA_FORMAT_R16G16_SINT,
  DATA_FORMAT_R24G8_TYPELESS,
  DATA_FORMAT_D24_UNORM_S8_UINT,
  DATA_FORMAT_R24_UNORM_X8_TYPELESS,
  DATA_FORMAT_X24_TYPELESS_G8_UINT,
  DATA_FORMAT_FOUR_BYTE_TWO_CHANNEL_LAST, // R16G16 , D24S8
  DATA_FORMAT_R11G11B10_FLOAT,
  DATA_FORMAT_FOUR_BYTE_THREE_CHANNEL_LAST, // R11G11B10
  DATA_FORMAT_R8G8B8A8_TYPELESS,
  DATA_FORMAT_R8G8B8A8_UNORM,
  DATA_FORMAT_R8G8B8A8_UNORM_SRGB,
  DATA_FORMAT_R8G8B8A8_UINT,
  DATA_FORMAT_R8G8B8A8_SNORM,
  DATA_FORMAT_R8G8B8A8_SINT,
  DATA_FORMAT_B8G8R8A8_UNORM,
  DATA_FORMAT_B8G8R8X8_UNORM,
  DATA_FORMAT_B8G8R8A8_TYPELESS,
  DATA_FORMAT_B8G8R8A8_UNORM_SRGB,
  DATA_FORMAT_B8G8R8X8_TYPELESS,
  DATA_FORMAT_B8G8R8X8_UNORM_SRGB,
  DATA_FORMAT_R10G10B10A2_TYPELESS,
  DATA_FORMAT_R10G10B10A2_UNORM,
  DATA_FORMAT_R10G10B10A2_UINT,
  DATA_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
  DATA_FORMAT_R8G8_B8G8_UNORM,
  DATA_FORMAT_G8R8_G8B8_UNORM,
  DATA_FORMAT_R9G9B9E5_SHAREDEXP,
  DATA_FORMAT_FOUR_BYTE_FOUR_CHANNEL_LAST, // R8G8B8A8 , R10B10G10A2,R9G9B9E5
  DATA_FORMAT_FOUR_BYTE_LAST,
  DATA_FORMAT_R32G32_TYPELESS,
  DATA_FORMAT_R32G32_FLOAT,
  DATA_FORMAT_R32G32_UINT,
  DATA_FORMAT_R32G32_SINT,
  DATA_FORMAT_EIGHT_BYTE_TWO_CHANNEL_LAST, // R32B32
  DATA_FORMAT_R32G8X24_TYPELESS,
  DATA_FORMAT_D32_FLOAT_S8X24_UINT,
  DATA_FORMAT_R32_FLOAT_X8X24_TYPELESS,
  DATA_FORMAT_X32_TYPELESS_G8X24_UINT,
  DATA_FORMAT_EIGHT_BYTE_THREE_CHANNEL_LAST, // R32G8X24
  DATA_FORMAT_R16G16B16A16_TYPELESS,
  DATA_FORMAT_R16G16B16A16_FLOAT,
  DATA_FORMAT_R16G16B16A16_UNORM,
  DATA_FORMAT_R16G16B16A16_UINT,
  DATA_FORMAT_R16G16B16A16_SNORM,
  DATA_FORMAT_R16G16B16A16_SINT,
  DATA_FORMAT_EIGHT_BYTE_FOUR_CHANNEL_LAST, // R16G16B16A16
  DATA_FORMAT_EIGHT_BYTE_LAST,
  DATA_FORMAT_R32G32B32_TYPELESS,
  DATA_FORMAT_R32G32B32_FLOAT,
  DATA_FORMAT_R32G32B32_UINT,
  DATA_FORMAT_R32G32B32_SINT,
  DATA_FORMAT_TWLEVE_BYTE_LAST, // only R32B32G32
  DATA_FORMAT_R32G32B32A32_TYPELESS,
  DATA_FORMAT_R32G32B32A32_FLOAT,
  DATA_FORMAT_R32G32B32A32_UINT,
  DATA_FORMAT_R32G32B32A32_SINT,
  DATA_FORMAT_SIXTEEN_BYTE_LAST, // only R32B32G32A32
};
inline std::pair<unsigned int, unsigned int>
getDataFormatByteChannelSize(DataFormat format) {
  switch (format) {
  case DataFormat::DATA_FORMAT_UNKNOWN:
    return {0, 0};
  case DataFormat::DATA_FORMAT_ONE_BYTE_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_TWO_BYTE_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_TWO_BYTE_ONE_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_TWO_BYTE_TWO_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_FOUR_BYTE_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_FOUR_BYTE_ONE_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_FOUR_BYTE_TWO_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_FOUR_BYTE_THREE_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_FOUR_BYTE_FOUR_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_EIGHT_BYTE_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_EIGHT_BYTE_TWO_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_EIGHT_BYTE_THREE_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_EIGHT_BYTE_FOUR_CHANNEL_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_TWLEVE_BYTE_LAST:
    return {0, 0};
  case DataFormat::DATA_FORMAT_SIXTEEN_BYTE_LAST:
    return {0, 0};
  default:
    break;
  }
  if (format < DataFormat::DATA_FORMAT_ONE_BYTE_LAST)
    return {1, 1};
  if (format < DataFormat::DATA_FORMAT_TWO_BYTE_LAST) {
    if (format < DataFormat::DATA_FORMAT_TWO_BYTE_ONE_CHANNEL_LAST)
      return {2, 1};
    if (format < DataFormat::DATA_FORMAT_TWO_BYTE_TWO_CHANNEL_LAST)
      return {2, 2};
  }
  if (format < DataFormat::DATA_FORMAT_FOUR_BYTE_LAST) {
    if (format < DataFormat::DATA_FORMAT_FOUR_BYTE_ONE_CHANNEL_LAST)
      return {4, 1};
    if (format < DataFormat::DATA_FORMAT_FOUR_BYTE_TWO_CHANNEL_LAST)
      return {4, 2};
    if (format < DataFormat::DATA_FORMAT_FOUR_BYTE_THREE_CHANNEL_LAST)
      return {4, 3};
    if (format < DataFormat::DATA_FORMAT_FOUR_BYTE_FOUR_CHANNEL_LAST)
      return {4, 4};
  }
  if (format < DataFormat::DATA_FORMAT_EIGHT_BYTE_LAST) {
    if (format < DataFormat::DATA_FORMAT_EIGHT_BYTE_TWO_CHANNEL_LAST)
      return {8, 2};
    if (format < DataFormat::DATA_FORMAT_EIGHT_BYTE_THREE_CHANNEL_LAST)
      return {8, 3};
    if (format < DataFormat::DATA_FORMAT_EIGHT_BYTE_FOUR_CHANNEL_LAST)
      return {8, 4};
  }
  if (format < DataFormat::DATA_FORMAT_TWLEVE_BYTE_LAST) {
    return {12, 3};
  }
  if (format < DataFormat::DATA_FORMAT_SIXTEEN_BYTE_LAST) {
    return {16, 4};
  }
  return {0, 0};
}
inline unsigned int getDataFormatByteSize(DataFormat format) {
  return getDataFormatByteChannelSize(format).first;
}
inline unsigned int getDataFormatChannelSize(DataFormat format) {
  return getDataFormatByteChannelSize(format).second;
}
inline bool hasDepthFormat(DataFormat format) {
  switch (format) {
  case DataFormat::DATA_FORMAT_D16_UNORM:
    return true;
  case DataFormat::DATA_FORMAT_D24_UNORM_S8_UINT:
    return true;
  case DataFormat::DATA_FORMAT_D32_FLOAT:
    return true;
  case DataFormat::DATA_FORMAT_D32_FLOAT_S8X24_UINT:
    return true;
  }
  return false;
}
inline bool hasStencilFormat(DataFormat format) {
  switch (format) {
  case DataFormat::DATA_FORMAT_D24_UNORM_S8_UINT:
    return true;
  case DataFormat::DATA_FORMAT_D32_FLOAT_S8X24_UINT:
    return true;
  }
  return false;
}
enum class RawFormat {
  RAW_FORMAT_UNKNOWN,
  RAW_FORMAT_R8,
  RAW_FORMAT_R16,
  RAW_FORMAT_R8G8,
  RAW_FORMAT_R32,
  RAW_FORMAT_R16G16,
  RAW_FORMAT_R24G8,
  RAW_FORMAT_R8G8B8A8,
  RAW_FORMAT_B8G8R8A8,
  RAW_FORMAT_B8G8R8X8,
  RAW_FORMAT_R10G10B10A2,
  RAW_FORMAT_R32G32,
  RAW_FORMAT_R32G8X24,
  RAW_FORMAT_R16G16B16A16,
  RAW_FORMAT_R32G32B32,
  RAW_FORMAT_R32G32B32A32,
};
inline DataFormat convertToDataFormat(RawFormat format) {
  switch (format) {
  case RawFormat::RAW_FORMAT_R8:
    return DataFormat::DATA_FORMAT_R8_TYPELESS;
  case RawFormat::RAW_FORMAT_R16:
    return DataFormat::DATA_FORMAT_R16_TYPELESS;
  case RawFormat::RAW_FORMAT_R8G8:
    return DataFormat::DATA_FORMAT_R8G8_TYPELESS;
  case RawFormat::RAW_FORMAT_R32:
    return DataFormat::DATA_FORMAT_R32_TYPELESS;
  case RawFormat::RAW_FORMAT_R16G16:
    return DataFormat::DATA_FORMAT_R16G16_TYPELESS;
  case RawFormat::RAW_FORMAT_R24G8:
    return DataFormat::DATA_FORMAT_R24G8_TYPELESS;
  case RawFormat::RAW_FORMAT_R8G8B8A8:
    return DataFormat::DATA_FORMAT_R8G8B8A8_TYPELESS;
  case RawFormat::RAW_FORMAT_B8G8R8A8:
    return DataFormat::DATA_FORMAT_B8G8R8A8_TYPELESS;
  case RawFormat::RAW_FORMAT_B8G8R8X8:
    return DataFormat::DATA_FORMAT_B8G8R8X8_TYPELESS;
  case RawFormat::RAW_FORMAT_R10G10B10A2:
    return DataFormat::DATA_FORMAT_R10G10B10A2_TYPELESS;
  case RawFormat::RAW_FORMAT_R32G32:
    return DataFormat::DATA_FORMAT_R32G32_TYPELESS;
  case RawFormat::RAW_FORMAT_R32G8X24:
    return DataFormat::DATA_FORMAT_R32G8X24_TYPELESS;
  case RawFormat::RAW_FORMAT_R16G16B16A16:
    return DataFormat::DATA_FORMAT_R16G16B16A16_TYPELESS;
  case RawFormat::RAW_FORMAT_R32G32B32:
    return DataFormat::DATA_FORMAT_R32G32B32_TYPELESS;
  case RawFormat::RAW_FORMAT_R32G32B32A32:
    return DataFormat::DATA_FORMAT_R32G32B32A32_TYPELESS;
  }
  return DataFormat::DATA_FORMAT_UNKNOWN;
}
enum class DepthStencilFormat {
  DEPTH_STENCIL_FORMAT_UNKNOWN,
  DEPTH_STENCIL_FORMAT_D16_UNORM,
  DEPTH_STENCIL_FORMAT_D32_FLOAT,
  DEPTH_STENCIL_FORMAT_D24_UNORM_S8_UINT,
  DEPTH_STENCIL_FORMAT_D32_FLOAT_S8X24_UINT,
};
inline DataFormat convertToDataFormat(DepthStencilFormat format) {
  switch (format) {
  case DepthStencilFormat::DEPTH_STENCIL_FORMAT_D16_UNORM:
    return DataFormat::DATA_FORMAT_D16_UNORM;
  case DepthStencilFormat::DEPTH_STENCIL_FORMAT_D24_UNORM_S8_UINT:
    return DataFormat::DATA_FORMAT_D24_UNORM_S8_UINT;
  case DepthStencilFormat::DEPTH_STENCIL_FORMAT_D32_FLOAT:
    return DataFormat::DATA_FORMAT_D32_FLOAT;
  case DepthStencilFormat::DEPTH_STENCIL_FORMAT_D32_FLOAT_S8X24_UINT:
    return DataFormat::DATA_FORMAT_D32_FLOAT_S8X24_UINT;
  case DepthStencilFormat::DEPTH_STENCIL_FORMAT_UNKNOWN:
    return DataFormat::DATA_FORMAT_UNKNOWN;
  }
  return DataFormat::DATA_FORMAT_UNKNOWN;
}
enum class BindUsage {
  BIND_USAGE_CBV,
  BIND_USAGE_SRV,
  BIND_USAGE_UAV,
  BIND_USAGE_SAMPLER,
  BIND_USAGE_UNKNOWN,
};
enum class ResourceUsage {
  RESOURCE_USAGE_CBV,
  RESOURCE_USAGE_SRV,
  RESOURCE_USAGE_UAV,
  RESOURCE_USAGE_RTV,
  RESOURCE_USAGE_DSV,
  RESOURCE_USAGE_SAMPLER,
  RESOURCE_USAGE_UNKNOWN,
};
inline DescriptorType getDescriptorType(ResourceUsage usage) {
  switch (usage) {
  case ResourceUsage::RESOURCE_USAGE_CBV:
    return DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV;
  case ResourceUsage::RESOURCE_USAGE_SRV:
    return DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV;
  case ResourceUsage::RESOURCE_USAGE_UAV:
    return DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV;
  case ResourceUsage::RESOURCE_USAGE_RTV:
    return DescriptorType::DESCRIPTOR_TYPE_RTV;
  case ResourceUsage::RESOURCE_USAGE_DSV:
    return DescriptorType::DESCRIPTOR_TYPE_DSV;
  case ResourceUsage::RESOURCE_USAGE_SAMPLER:
    return DescriptorType::DESCRIPTOR_TYPE_SAMPLER;
  }
  return DescriptorType::DESCRIPTOR_TYPE_COUNT;
}
enum class ShaderType {
  SHADER_TYPE_NONE,
  SHADER_TYPE_VERTEX,
  SHADER_TYPE_PIXEL,
  SHADER_TYPE_GEOMETRY,
  SHADER_TYPE_COMPUTE,
  SHADER_TYPE_HULL,
  SHADER_TYPE_DOMAIN,
  SHADER_TYPE_AMPLIFICATION,
  SHADER_TYPE_MESH,
  SHADER_TYPE_ALL, // only for shader visibility
};
enum class BindType : unsigned {
  BIND_TYPE_SIT_CBUFFER = 0x01,
  BIND_TYPE_CBV_BOUND = 0x02,
  BIND_TYPE_SIT_TBUFFER = 0x04,
  BIND_TYPE_SIT_STRUCTURED = 0x08,
  BIND_TYPE_SIT_BYTEADDRESS = 0x10,
  BIND_TYPE_SIT_TEXTURE = 0x20,
  BIND_TYPE_SRV_BOUND = 0x40,
  BIND_TYPE_SIT_UAV_RWTYPED = 0x80,
  BIND_TYPE_SIT_UAV_RWSTRUCTURED = 0x100,
  BIND_TYPE_SIT_UAV_RWBYTEADDRESS = 0x200,
  BIND_TYPE_SIT_UAV_APPEND_STRUCTURED = 0x400,
  BIND_TYPE_SIT_UAV_CONSUME_STRUCTURED = 0x800,
  BIND_TYPE_SIT_UAV_RWSTRUCTURED_WITH_COUNTER = 0x1000,
  BIND_TYPE_SIT_UAV_FEEDBACKTEXTURE = 0x2000,
  BIND_TYPE_UAV_BOUND = 0x4000,
  BIND_TYPE_SIT_SAMPLER = 0x8000,
  BIND_TYPE_SIT_RTACCELERATIONSTRUCTURE = 0x10000,
  BIND_TYPE_SIT_ROOT_CONSTANT =
      0x20000, // will not get by the shader, should set manually
  BIND_TYPE_SIT_ALL = 0xfffff, // only use for query from shader
};
inline BindType operator|(const BindType &lhs, const BindType &rhs) {
  return static_cast<BindType>(
      static_cast<std::underlying_type<BindType>::type>(lhs) |
      static_cast<std::underlying_type<BindType>::type>(rhs));
}
inline BindType operator&(const BindType &lhs, const BindType &rhs) {
  return static_cast<BindType>(
      static_cast<std::underlying_type<BindType>::type>(lhs) &
      static_cast<std::underlying_type<BindType>::type>(rhs));
}

inline BindUsage getUsage(BindType type) {
  if (type < BindType::BIND_TYPE_CBV_BOUND)
    return BindUsage::BIND_USAGE_CBV;
  if (type < BindType::BIND_TYPE_SRV_BOUND)
    return BindUsage::BIND_USAGE_SRV;
  if (type < BindType::BIND_TYPE_UAV_BOUND)
    return BindUsage::BIND_USAGE_UAV;
  if (type == BindType::BIND_TYPE_SIT_SAMPLER)
    return BindUsage::BIND_USAGE_SAMPLER;
  return BindUsage::BIND_USAGE_UNKNOWN;
}
enum class DataDimension {
  DATA_DIMENSION_UNKNOWN,
  DATA_DIMENSION_BUFFER,
  DATA_DIMENSION_TEXTURE1D,
  DATA_DIMENSION_TEXTURE1DARRAY,
  DATA_DIMENSION_TEXTURE2D,
  DATA_DIMENSION_TEXTURE2DARRAY,
  DATA_DIMENSION_TEXTURE2DMS,
  DATA_DIMENSION_TEXTURE2DMSARRAY,
  DATA_DIMENSION_TEXTURE3D,
  DATA_DIMENSION_TEXTURECUBE,
  DATA_DIMENSION_TEXTURECUBEARRAY,
  DATA_DIMENSION_BUFFEREX,
};
using BindSignature = ID3D12RootSignature;
enum class Blend {
  BLEND_ZERO,
  BLEND_ONE,
  BLEND_SRC_COLOR,
  BLEND_INV_SRC_COLOR,
  BLEND_SRC_ALPHA,
  BLEND_INV_SRC_ALPHA,
  BLEND_DEST_ALPHA,
  BLEND_INV_DEST_ALPHA,
  BLEND_DEST_COLOR,
  BLEND_INV_DEST_COLOR,
  BLEND_SRC_ALPHA_SAT,
  BLEND_BLEND_FACTOR,
  BLEND_INV_BLEND_FACTOR,
  BLEND_SRC1_COLOR,
  BLEND_INV_SRC1_COLOR,
  BLEND_SRC1_ALPHA,
  BLEND_INV_SRC1_ALPHA
};
enum class BlendOperation {
  BLEND_OPERATION_ADD,
  BLEND_OPERATION_SUBTRACT,
  BLEND_OPERATION_REV_SUBTRACT,
  BLEND_OPERATION_MIN,
  BLEND_OPERATION_MAX
};
enum class LogicOperation {
  LOGIC_OPERATION_CLEAR,
  LOGIC_OPERATION_SET,
  LOGIC_OPERATION_COPY,
  LOGIC_OPERATION_COPY_INVERTED,
  LOGIC_OPERATION_NOOP,
  LOGIC_OPERATION_INVERT,
  LOGIC_OPERATION_AND,
  LOGIC_OPERATION_NAND,
  LOGIC_OPERATION_OR,
  LOGIC_OPERATION_NOR,
  LOGIC_OPERATION_XOR,
  LOGIC_OPERATION_EQUIV,
  LOGIC_OPERATION_AND_REVERSE,
  LOGIC_OPERATION_AND_INVERTED,
  LOGIC_OPERATION_OR_REVERSE,
  LOGIC_OPERATION_OR_INVERTED,
};
enum class ColorWriteMask : unsigned char {
  COLOR_WRITE_MASK_RED = 0x1,
  COLOR_WRITE_MASK_GREEN = 0x2,
  COLOR_WRITE_MASK_BLUE = 0x4,
  COLOR_WRITE_MASK_ALPHA = 0x8,
  COLOR_WRITE_MASK_ALL = 0xf,
};
inline ColorWriteMask operator|(const ColorWriteMask &lhs,
                                const ColorWriteMask &rhs) {
  return static_cast<ColorWriteMask>(
      static_cast<std::underlying_type<ColorWriteMask>::type>(lhs) |
      static_cast<std::underlying_type<ColorWriteMask>::type>(rhs));
}
static const unsigned int render_targets_limits_ = 8;
enum class DepthWriteMask {
  DEPTH_WRITE_MASK_ZERO = 0,
  DEPTH_WRITE_MASK_ALL = 1
};
enum class ComparisonFunction {
  COMPARISON_FUNCTION_NEVER,
  COMPARISON_FUNCTION_LESS,
  COMPARISON_FUNCTION_EQUAL,
  COMPARISON_FUNCTION_LESS_EQUAL,
  COMPARISON_FUNCTION_GREATER,
  COMPARISON_FUNCTION_NOT_EQUAL,
  COMPARISON_FUNCTION_GREATER_EQUAL,
  COMPARISON_FUNCTION_ALWAYS
};
enum class StencilOperation {
  STENCIL_OPERATION_KEEP,
  STENCIL_OPERATION_ZERO,
  STENCIL_OPERATION_REPLACE,
  STENCIL_OPERATION_INCR_SAT,
  STENCIL_OPERATION_DECR_SAT,
  STENCIL_OPERATION_INVERT,
  STENCIL_OPERATION_INCR,
  STENCIL_OPERATION_DECR
};
static const unsigned char default_stencil_read_mask_ =
    D3D12_DEFAULT_STENCIL_READ_MASK;
static const unsigned char default_stencil_write_mask_ =
    D3D12_DEFAULT_STENCIL_WRITE_MASK;
enum class FillMode {
  FILL_MODE_WIREFRAME,
  FILL_MODE_SOLID,
};
enum class CullMode { CULL_MODE_NONE, CULL_MODE_FRONT, CULL_MODE_BACK };
enum class PrimitiveTopologyType {
  PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,
  PRIMITIVE_TOPOLOGY_TYPE_POINT,
  PRIMITIVE_TOPOLOGY_TYPE_LINE,
  PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
  PRIMITIVE_TOPOLOGY_TYPE_PATCH,
};

using PipelineState = ID3D12PipelineState;
const static float default_max_depth_ = D3D12_MAX_DEPTH;
const static float default_min_depth_ = D3D12_MIN_DEPTH;
using ViewportBase = D3D12_VIEWPORT;
using ScissorBase = D3D12_RECT;
enum class PrimitiveTopology {
  PRIMITIVE_TOPOLOGY_UNDEFINED,
  PRIMITIVE_TOPOLOGY_POINTLIST,
  PRIMITIVE_TOPOLOGY_LINELIST,
  PRIMITIVE_TOPOLOGY_LINESTRIP,
  PRIMITIVE_TOPOLOGY_TRIANGLELIST,
  PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
  PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
  PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,
  PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
  PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
  PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST,
  PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST,
};
static unsigned int counter_buffer_alignment_ =
    D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
static unsigned int constant_buffer_alignment_ = 256;
static unsigned int count_size_ = sizeof(unsigned int);

enum class BufferType {
  BUFFER_TYPE_NONE = 0x0,
  BUFFER_TYPE_VERTEX = 0x1,
  BUFFER_TYPE_INDEX = 0x2,
  BUFFER_TYPE_CONSTANT = 0x4,
  BUFFER_TYPE_STRUCTERED = 0x8,
  BUFFER_TYPE_COUNTER = 0x10,
  BUFFER_TYPE_RAW = 0x20,
  BUFFER_TYPE_CUSTOM = 0xff,
};
enum class TextureType { TEXTURE_TYPE_1D, TEXTURE_TYPE_2D, TEXTURE_TYPE_3D };
// only used when the texture usage  and rtv, dsv dimension is unknown
// 1 with simple texture, >1 with arrays, cube and MS should always be
// decleared by user
inline DataDimension getDataDimension(TextureType type, unsigned int depth) {
  switch (type) {
  case TextureType::TEXTURE_TYPE_1D:
    if (depth == 1)
      return DataDimension::DATA_DIMENSION_TEXTURE1D;
    else
      return DataDimension::DATA_DIMENSION_TEXTURE1DARRAY;
  case TextureType::TEXTURE_TYPE_2D:
    if (depth == 1)
      return DataDimension::DATA_DIMENSION_TEXTURE2D;
    else
      return DataDimension::DATA_DIMENSION_TEXTURE2DARRAY;
  case TextureType::TEXTURE_TYPE_3D:
    return DataDimension::DATA_DIMENSION_TEXTURE3D;
  }
  return DataDimension::DATA_DIMENSION_UNKNOWN;
}
enum class DepthStencilFlags {
  DEPTH_SENCIL_FLAG_NONE = 0x0,
  DEPTH_SENCIL_FLAG_READ_ONLY_DEPTH = 0x1,
  DEPTH_SENCIL_FLAG_READ_ONLY_STENCIL = 0x2,
  DEPTH_SENCIL_FLAG_READ_ONLY = 0x3,
};
// should have buffer usage and texture usage
// will have a siimple check whether it is right dimension
// buffer, the element size and element szie should be
// provided when create buffer
struct BufferUsage {
  ResourceUsage usage_ = ResourceUsage::RESOURCE_USAGE_UNKNOWN;
  unsigned long long start_index_ = 0;
  bool is_writable_ = false;
  bool uav_use_counter_ = false;
  bool is_raw_buffer_ = false;
};
using MipSlice = unsigned int;
struct MipRange {
  unsigned int mips_start_level_ = 0;
  float lod_lower_bound_ = 0.0f;
  unsigned int mips_count_ = (std::numeric_limits<unsigned int>::max)();
};
// count_ will be have a min(depth,count) when create
// desc, -1 is good for 3d, for cube, this is first
// face start and cube counts will become min((depth-array_index)/6,count)
struct SubTexturesRange {
  unsigned int array_start_index_ = 0;
  unsigned int count_ = (std::numeric_limits<unsigned int>::max)();
};
// take a look at d3d12 array and mip slice
// https://docs.microsoft.com/en-us/windows/win32/direct3d12/subresources
// this is just a srv/dsv/rtv combined descriptor

// if dimension is unknown, then it will follow the texture type
// and check the depth, if depth > 1, it will be arrays

struct TextureUsage {
  ResourceUsage usage_ = ResourceUsage::RESOURCE_USAGE_UNKNOWN;
  DataFormat data_format_ = DataFormat::DATA_FORMAT_UNKNOWN;
  DataDimension data_dimension_ = DataDimension::DATA_DIMENSION_UNKNOWN;
  MipRange mip_range_;
  MipSlice mip_slice_ = 0;
  SubTexturesRange sub_texture_ranges_;
  unsigned int plane_slice_ = 0;
};

inline void getSubResourceList(std::vector<uint32_t> &list, TextureUsage &usage,
                               uint32_t mip_levels, uint32_t depth_size) {
  unsigned int array_start = usage.sub_texture_ranges_.array_start_index_;
  unsigned int array_size =
      (std::min)(usage.sub_texture_ranges_.count_, depth_size - array_start);
  if (usage.usage_ == ResourceUsage::RESOURCE_USAGE_SRV) {
    if (usage.data_dimension_ == DataDimension::DATA_DIMENSION_TEXTURE3D) {
      array_size = 1;
      array_start = 0;
    }
    uint32_t total_levels =
        (std::min)(usage.mip_range_.mips_count_,
                   mip_levels - usage.mip_range_.mips_start_level_);
    for (unsigned int i = 0; i < array_size; ++i) {
      for (unsigned int j = 0; j < total_levels; ++j) {
        unsigned int index = (i + array_start) * mip_levels + j +
                             usage.mip_range_.mips_start_level_;
        list.push_back(index);
      }
    }
  } else {
    if (usage.data_dimension_ == DataDimension::DATA_DIMENSION_TEXTURE3D) {
      array_size = 1;
      array_start = 0;
    }
    for (unsigned int i = 0; i < array_size; ++i) {
      unsigned int index = (i + array_start) * mip_levels + usage.mip_slice_;
      list.push_back(index);
    }
  }
}
struct RenderTargetUsage {
  DataFormat data_format_ = DataFormat::DATA_FORMAT_UNKNOWN;
  DataDimension data_dimension_ = DataDimension::DATA_DIMENSION_UNKNOWN;
  MipSlice mip_slice_ = 0;
  SubTexturesRange sub_texture_ranges_;
  unsigned int plane_slice_ = 0;
};
inline void getRenderTargetSubResourceList(std::vector<uint32_t> &list,
                                           RenderTargetUsage &usage,
                                           uint32_t mip_levels,
                                           uint32_t depth_size) {
  unsigned int array_start = usage.sub_texture_ranges_.array_start_index_;
  unsigned int array_size =
      (std::min)(usage.sub_texture_ranges_.count_, depth_size - array_start);
  if (usage.data_dimension_ == DataDimension::DATA_DIMENSION_TEXTURE3D) {
    array_size = 1;
    array_start = 0;
  }
  for (unsigned int i = 0; i < array_size; ++i) {
    unsigned int index = (i + array_start) * mip_levels + usage.mip_slice_;
    list.push_back(index);
  }
}

static const std::vector<RenderTargetUsage> empty_render_target_usage;
struct DepthStencilUsage {
  DepthStencilFormat data_format_ =
      DepthStencilFormat::DEPTH_STENCIL_FORMAT_UNKNOWN;
  DataDimension data_dimension_ = DataDimension::DATA_DIMENSION_UNKNOWN;
  MipSlice mip_slice_ = 0;
  SubTexturesRange sub_texture_ranges_;
  unsigned int plane_slice_ = 0;
  DepthStencilFlags depth_stencil_flag_ =
      DepthStencilFlags::DEPTH_SENCIL_FLAG_NONE;
};
inline void geDepthStencilSubResourceList(std::vector<uint32_t> &list,
                                          DepthStencilUsage &usage,
                                          uint32_t mip_levels,
                                          uint32_t depth_size) {
  unsigned int array_start = usage.sub_texture_ranges_.array_start_index_;
  unsigned int array_size =
      (std::min)(usage.sub_texture_ranges_.count_, depth_size - array_start);
  for (unsigned int i = 0; i < array_size; ++i) {
    unsigned int index = (i + array_start) * mip_levels + usage.mip_slice_;
    list.push_back(index);
  }
}

static const std::vector<DepthStencilUsage> empty_depth_stencil_usage;
using TextureFootPrint = D3D12_PLACED_SUBRESOURCE_FOOTPRINT;
static unsigned long long texture_subresouce_offset_aligment = 512U;
// for box update, need to think more
struct SourceTextureBox {
  const void *data_;
  // the offset and size of the data need to be copy
  unsigned long long left_ = 0;
  unsigned long long right_ = 1;
  unsigned int top_ = 0;
  unsigned int bottom_ = 1;
  unsigned int font_ = 0;
  unsigned int back_ = 1;
  // if width or height or depth means the data is consecative
  // don't care about the width, height, depth
  unsigned long long data_width_ =
      (std::numeric_limits<unsigned long long>::max)();
  unsigned int data_height_ = (std::numeric_limits<unsigned int>::max)();
  unsigned int data_depth_ = (std::numeric_limits<unsigned int>::max)();
};
struct DestinyTextureBox {
  unsigned long long left_ = 0;
  unsigned int top_ = 0;
  unsigned int font_ = 0;
};
enum class Filter {
  FILTER_MIN_MAG_MIP_POINT = 0,
  FILTER_MIN_MAG_POINT_MIP_LINEAR = 0x1,
  FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
  FILTER_MIN_POINT_MAG_MIP_LINEAR = 0x5,
  FILTER_MIN_LINEAR_MAG_MIP_POINT = 0x10,
  FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
  FILTER_MIN_MAG_LINEAR_MIP_POINT = 0x14,
  FILTER_MIN_MAG_MIP_LINEAR = 0x15,
  FILTER_ANISOTROPIC = 0x55,
  FILTER_COMPARISON_MIN_MAG_MIP_POINT = 0x80,
  FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 0x81,
  FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x84,
  FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 0x85,
  FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 0x90,
  FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x91,
  FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 0x94,
  FILTER_COMPARISON_MIN_MAG_MIP_LINEAR = 0x95,
  FILTER_COMPARISON_ANISOTROPIC = 0xd5,
  FILTER_MINIMUM_MIN_MAG_MIP_POINT = 0x100,
  FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x101,
  FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x104,
  FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x105,
  FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x110,
  FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x111,
  FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x114,
  FILTER_MINIMUM_MIN_MAG_MIP_LINEAR = 0x115,
  FILTER_MINIMUM_ANISOTROPIC = 0x155,
  FILTER_MAXIMUM_MIN_MAG_MIP_POINT = 0x180,
  FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x181,
  FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x184,
  FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x185,
  FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x190,
  FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x191,
  FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x194,
  FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR = 0x195,
  FILTER_MAXIMUM_ANISOTROPIC = 0x1d5
};
enum class TextureAddressMode {
  TEXTURE_ADDRESS_MODE_WRAP = 1,
  TEXTURE_ADDRESS_MODE_MIRROR = 2,
  TEXTURE_ADDRESS_MODE_CLAMP = 3,
  TEXTURE_ADDRESS_MODE_BORDER = 4,
  TEXTURE_ADDRESS_MODE_MIRROR_ONCE = 5
};
inline ResourceState
getBindState(ResourceUsage usage,
             ShaderType visibility = ShaderType::SHADER_TYPE_ALL) {
  switch (usage) {
  case ResourceUsage::RESOURCE_USAGE_CBV:
    return ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  case ResourceUsage::RESOURCE_USAGE_UAV:
    return ResourceState::RESOURCE_STATE_UNORDERED_ACCESS;
  case ResourceUsage::RESOURCE_USAGE_SRV:
    if (visibility == ShaderType::SHADER_TYPE_ALL)
      return ResourceState::RESOURCE_STATE_ALL_SHADER_RESOURCE;
    if (visibility == ShaderType::SHADER_TYPE_PIXEL)
      return ResourceState::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    return ResourceState::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
  }
  return ResourceState::RESOURCE_STATE_COMMON;
}
enum class TranstionState {
  TRANSITION_STATE_DONE,
  TRANSITION_STATE_TRANSITING
};
// when next state is not unknown, means it's under split barrier
struct SubResourceState {
  ResourceState previous_state_ = ResourceState::RESOURCE_STATE_UNKNOWN;
  ResourceState current_state_ = ResourceState::RESOURCE_STATE_UNKNOWN;
  bool unUsed() {
    return previous_state_ == ResourceState::RESOURCE_STATE_UNKNOWN ||
           current_state_ == ResourceState::RESOURCE_STATE_UNKNOWN;
  }
  bool isTransiting() { return !(previous_state_ == current_state_); }
  bool operator==(const SubResourceState &rhs) const {
    return previous_state_ == rhs.previous_state_ &&
           current_state_ == rhs.current_state_;
  }
  bool operator!=(const SubResourceState &rhs) const { return !(*this == rhs); }
};
inline ResourceState getStateFromBindTypeVisibility(BindType bind_type,
                                                    ShaderType visibility) {
  BindUsage usage = getUsage(bind_type);
  if (usage == BindUsage::BIND_USAGE_CBV)
    return ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  if (usage == BindUsage::BIND_USAGE_SRV) {
    if (visibility == ShaderType::SHADER_TYPE_ALL)
      return ResourceState::RESOURCE_STATE_ALL_SHADER_RESOURCE;
    if (visibility == ShaderType::SHADER_TYPE_PIXEL)
      return ResourceState::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    return ResourceState::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
  }
  if (usage == BindUsage::BIND_USAGE_UAV)
    return ResourceState::RESOURCE_STATE_UNORDERED_ACCESS;
  return ResourceState::RESOURCE_STATE_UNKNOWN;
}
inline bool checkBindTypeUsageType(BindType bind_type, ResourceUsage usage) {
  // BindUsage
  BindUsage bind_usage = getUsage(bind_type);
  switch (bind_usage) {
  case BindUsage::BIND_USAGE_CBV:
    return usage == ResourceUsage::RESOURCE_USAGE_CBV;
  case BindUsage::BIND_USAGE_SRV:
    return usage == ResourceUsage::RESOURCE_USAGE_SRV;
  case BindUsage::BIND_USAGE_UAV:
    return usage == ResourceUsage::RESOURCE_USAGE_UAV;
  case BindUsage::BIND_USAGE_SAMPLER:
    return usage == ResourceUsage::RESOURCE_USAGE_SAMPLER;
  }
  return false;
}
namespace Resource {
class Resource;
}
struct Transition {
  /*Transition(std::shared_ptr<Resource::Resource> &resource)
      : resource_(resource) {}*/
  Resource::Resource *resource_;
  ResourceState before_state_;
  ResourceState after_state_;
  ResourceTransitionFlag flag_;
  unsigned int subresource_index_;
};
} // namespace Renderer
} // namespace CHCEngine