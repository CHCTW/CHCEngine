#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

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
#define NAME_D3D12_OBJECT_STRING(x, name)\
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
enum class ResourceState {
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
  RESOURCE_STATE_VIDEO_ENCODE_WRITE = 0x800000
};

enum class ResourceTransitionFlag {
  RESOURCE_TRANSITION_FLAG_NONE = 0,
  RESOURCE_TRANSITION_FLAG_BEGIN = 0x1,
  RESOURCE_TRANSITION_FLAG_END = 0x2
};
static unsigned int all_subresrouce_index =
    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
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
enum class BindUsage {
  BIND_USAGE_CBV,
  BIND_USAGE_SRV,
  BIND_USAGE_UAV,
  BIND_USAGE_SAMPLER,
  BIND_USAGE_UNKNOWN,
};

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
  SHADER_TYPE_ALL, // only for shader visiblity
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
enum class CullMode {
  CULL_MODE_NONE,
  CULL_MODE_FRONT,
  CULL_MODE_BACK
};
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
} // namespace Renderer
} // namespace CHCEngine