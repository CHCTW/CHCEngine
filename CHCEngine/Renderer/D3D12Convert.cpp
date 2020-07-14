#include "ClassName.h"

#include "D3D12Convert.h"
#include <magic_enum.hpp>
namespace CHCEngine {
namespace Renderer {
D3D12_DESCRIPTOR_HEAP_TYPE convertToD3DDescriptorHeapType(DescriptorType type) {
  switch (type) {
  case DescriptorType::DESCRIPTOR_TYPE_SRV_UAV_CBV:
    return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  case DescriptorType::DESCRIPTOR_TYPE_SAMPLER:
    return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
  case DescriptorType::DESCRIPTOR_TYPE_DSV:
    return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  case DescriptorType::DESCRIPTOR_TYPE_RTV:
    return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  }
  return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
}
D3D12_COMMAND_LIST_TYPE convertToD3D12CommandType(CommandType type) {
  switch (type) {
  case CommandType::COMMAND_TYPE_GRAPHICS:
    return D3D12_COMMAND_LIST_TYPE_DIRECT;
  case CommandType::COMMAND_TYPE_COMPUTE:
    return D3D12_COMMAND_LIST_TYPE_COMPUTE;
  case CommandType::COMMAND_TYPE_COPY:
    return D3D12_COMMAND_LIST_TYPE_COPY;
  default:
    return D3D12_COMMAND_LIST_TYPE_DIRECT;
  }
  return D3D12_COMMAND_LIST_TYPE_DIRECT;
}
D3D12_RESOURCE_STATES convertToD3D12ResourceStates(ResourceState state) {
  switch (state) {
  case ResourceState::RESOURCE_STATE_COMMON:
    return D3D12_RESOURCE_STATE_COMMON;
  case ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER:
    return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  case ResourceState::RESOURCE_STATE_INDEX_BUFFER:
    return D3D12_RESOURCE_STATE_INDEX_BUFFER;
  case ResourceState::RESOURCE_STATE_RENDER_TARGET:
    return D3D12_RESOURCE_STATE_RENDER_TARGET;
  case ResourceState::RESOURCE_STATE_UNORDERED_ACCESS:
    return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
  case ResourceState::RESOURCE_STATE_DEPTH_WRITE:
    return D3D12_RESOURCE_STATE_DEPTH_WRITE;
  case ResourceState::RESOURCE_STATE_DEPTH_READ:
    return D3D12_RESOURCE_STATE_DEPTH_READ;
  case ResourceState::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE:
    return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
  case ResourceState::RESOURCE_STATE_PIXEL_SHADER_RESOURCE:
    return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  case ResourceState::RESOURCE_STATE_STREAM_OUT:
    return D3D12_RESOURCE_STATE_STREAM_OUT;
  case ResourceState::RESOURCE_STATE_INDIRECT_ARGUMENT:
    return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
  case ResourceState::RESOURCE_STATE_COPY_DEST:
    return D3D12_RESOURCE_STATE_COPY_DEST;
  case ResourceState::RESOURCE_STATE_COPY_SOURCE:
    return D3D12_RESOURCE_STATE_COPY_SOURCE;
  case ResourceState::RESOURCE_STATE_RESOLVE_DEST:
    return D3D12_RESOURCE_STATE_RESOLVE_DEST;
  case ResourceState::RESOURCE_STATE_RESOLVE_SOURCE:
    return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
  case ResourceState::RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE:
    return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
  case ResourceState::RESOURCE_STATE_SHADING_RATE_SOURCE:
    return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
  case ResourceState::RESOURCE_STATE_GENERIC_READ:
    return D3D12_RESOURCE_STATE_GENERIC_READ;
  case ResourceState::RESOURCE_STATE_ALL_SHADER_RESOURCE:
    return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
  case ResourceState::RESOURCE_STATE_VIDEO_DECODE_READ:
    return D3D12_RESOURCE_STATE_VIDEO_DECODE_READ;
  case ResourceState::RESOURCE_STATE_VIDEO_DECODE_WRITE:
    return D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;
  case ResourceState::RESOURCE_STATE_VIDEO_PROCESS_READ:
    return D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ;
  case ResourceState::RESOURCE_STATE_VIDEO_PROCESS_WRITE:
    return D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE;
  case ResourceState::RESOURCE_STATE_VIDEO_ENCODE_READ:
    return D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ;
  case ResourceState::RESOURCE_STATE_VIDEO_ENCODE_WRITE:
    return D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;
  }
  return D3D12_RESOURCE_STATE_COMMON;
}
D3D12_RESOURCE_BARRIER_FLAGS
convertToD3D12ResourceBarrierFlags(ResourceTransitionFlag flag) {
  switch (flag) {
  case ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_BEGIN:
    return D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
  case ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_END:
    return D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
  case ResourceTransitionFlag::RESOURCE_TRANSITION_FLAG_NONE:
    return D3D12_RESOURCE_BARRIER_FLAG_NONE;
  }
  return D3D12_RESOURCE_BARRIER_FLAG_NONE;
}
D3D12_HEAP_TYPE convertToD3D12HeapType(HeapType type) {
  switch (type) {
  case HeapType::HEAP_TYPE_DEFAULT:
    return D3D12_HEAP_TYPE_DEFAULT;
  case HeapType::HEAP_TYPE_UPLOAD:
    return D3D12_HEAP_TYPE_UPLOAD;
  case HeapType::HEAP_TYPE_READBACK:
    return D3D12_HEAP_TYPE_READBACK;
  case HeapType::HEAP_TYPE_CUSTOM:
    return D3D12_HEAP_TYPE_CUSTOM;
  }
  return D3D12_HEAP_TYPE_DEFAULT;
}
DXGI_FORMAT convertToDXGIFormat(IndexFormat format) {
  switch (format) {
  case IndexFormat::INDEX_FORMAT_16_UINT:
    return DXGI_FORMAT_R16_UINT;
  case IndexFormat::INDEX_FORMAT_32_UINT:
    return DXGI_FORMAT_R32_UINT;
  }
  return DXGI_FORMAT_UNKNOWN;
}
DXGI_FORMAT convertToDXGIFormat(DataFormat format) {
  switch (format) {
  case DataFormat::DATA_FORMAT_R8_TYPELESS:
    return DXGI_FORMAT_R8_TYPELESS;
  case DataFormat::DATA_FORMAT_R8_UNORM:
    return DXGI_FORMAT_R8_UNORM;
  case DataFormat::DATA_FORMAT_R8_UINT:
    return DXGI_FORMAT_R8_UINT;
  case DataFormat::DATA_FORMAT_R8_SNORM:
    return DXGI_FORMAT_R8_SNORM;
  case DataFormat::DATA_FORMAT_R8_SINT:
    return DXGI_FORMAT_R8_SINT;
  case DataFormat::DATA_FORMAT_A8_UNORM:
    return DXGI_FORMAT_A8_UNORM;
  case DataFormat::DATA_FORMAT_R16_TYPELESS:
    return DXGI_FORMAT_R16_TYPELESS;
  case DataFormat::DATA_FORMAT_R16_FLOAT:
    return DXGI_FORMAT_R16_FLOAT;
  case DataFormat::DATA_FORMAT_D16_UNORM:
    return DXGI_FORMAT_D16_UNORM;
  case DataFormat::DATA_FORMAT_R16_UNORM:
    return DXGI_FORMAT_R16_UNORM;
  case DataFormat::DATA_FORMAT_R16_UINT:
    return DXGI_FORMAT_R16_UINT;
  case DataFormat::DATA_FORMAT_R16_SNORM:
    return DXGI_FORMAT_R16_SNORM;
  case DataFormat::DATA_FORMAT_R16_SINT:
    return DXGI_FORMAT_R16_SINT;
  case DataFormat::DATA_FORMAT_R8G8_TYPELESS:
    return DXGI_FORMAT_R8G8_TYPELESS;
  case DataFormat::DATA_FORMAT_R8G8_UNORM:
    return DXGI_FORMAT_R8G8_UNORM;
  case DataFormat::DATA_FORMAT_R8G8_UINT:
    return DXGI_FORMAT_R8G8_UINT;
  case DataFormat::DATA_FORMAT_R8G8_SNORM:
    return DXGI_FORMAT_R8G8_SNORM;
  case DataFormat::DATA_FORMAT_R8G8_SINT:
    return DXGI_FORMAT_R8G8_SINT;
  case DataFormat::DATA_FORMAT_R32_TYPELESS:
    return DXGI_FORMAT_R32_TYPELESS;
  case DataFormat::DATA_FORMAT_D32_FLOAT:
    return DXGI_FORMAT_D32_FLOAT;
  case DataFormat::DATA_FORMAT_R32_FLOAT:
    return DXGI_FORMAT_R32_FLOAT;
  case DataFormat::DATA_FORMAT_R32_UINT:
    return DXGI_FORMAT_R32_UINT;
  case DataFormat::DATA_FORMAT_R32_SINT:
    return DXGI_FORMAT_R32_SINT;
  case DataFormat::DATA_FORMAT_R16G16_TYPELESS:
    return DXGI_FORMAT_R16G16_TYPELESS;
  case DataFormat::DATA_FORMAT_R16G16_FLOAT:
    return DXGI_FORMAT_R16G16_FLOAT;
  case DataFormat::DATA_FORMAT_R16G16_UNORM:
    return DXGI_FORMAT_R16G16_UNORM;
  case DataFormat::DATA_FORMAT_R16G16_UINT:
    return DXGI_FORMAT_R16G16_UINT;
  case DataFormat::DATA_FORMAT_R16G16_SNORM:
    return DXGI_FORMAT_R16G16_SNORM;
  case DataFormat::DATA_FORMAT_R16G16_SINT:
    return DXGI_FORMAT_R16G16_SINT;
  case DataFormat::DATA_FORMAT_R24G8_TYPELESS:
    return DXGI_FORMAT_R24G8_TYPELESS;
  case DataFormat::DATA_FORMAT_D24_UNORM_S8_UINT:
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
  case DataFormat::DATA_FORMAT_R24_UNORM_X8_TYPELESS:
    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  case DataFormat::DATA_FORMAT_X24_TYPELESS_G8_UINT:
    return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
  case DataFormat::DATA_FORMAT_R11G11B10_FLOAT:
    return DXGI_FORMAT_R11G11B10_FLOAT;
  case DataFormat::DATA_FORMAT_R8G8B8A8_TYPELESS:
    return DXGI_FORMAT_R8G8B8A8_TYPELESS;
  case DataFormat::DATA_FORMAT_R8G8B8A8_UNORM:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  case DataFormat::DATA_FORMAT_R8G8B8A8_UNORM_SRGB:
    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  case DataFormat::DATA_FORMAT_R8G8B8A8_UINT:
    return DXGI_FORMAT_R8G8B8A8_UINT;
  case DataFormat::DATA_FORMAT_R8G8B8A8_SNORM:
    return DXGI_FORMAT_R8G8B8A8_SNORM;
  case DataFormat::DATA_FORMAT_R8G8B8A8_SINT:
    return DXGI_FORMAT_R8G8B8A8_SINT;
  case DataFormat::DATA_FORMAT_B8G8R8A8_UNORM:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case DataFormat::DATA_FORMAT_B8G8R8X8_UNORM:
    return DXGI_FORMAT_B8G8R8X8_UNORM;
  case DataFormat::DATA_FORMAT_B8G8R8A8_TYPELESS:
    return DXGI_FORMAT_B8G8R8A8_TYPELESS;
  case DataFormat::DATA_FORMAT_B8G8R8A8_UNORM_SRGB:
    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
  case DataFormat::DATA_FORMAT_B8G8R8X8_TYPELESS:
    return DXGI_FORMAT_B8G8R8X8_TYPELESS;
  case DataFormat::DATA_FORMAT_B8G8R8X8_UNORM_SRGB:
    return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
  case DataFormat::DATA_FORMAT_R10G10B10A2_TYPELESS:
    return DXGI_FORMAT_R10G10B10A2_TYPELESS;
  case DataFormat::DATA_FORMAT_R10G10B10A2_UNORM:
    return DXGI_FORMAT_R10G10B10A2_UNORM;
  case DataFormat::DATA_FORMAT_R10G10B10A2_UINT:
    return DXGI_FORMAT_R10G10B10A2_UINT;
  case DataFormat::DATA_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
  case DataFormat::DATA_FORMAT_R8G8_B8G8_UNORM:
    return DXGI_FORMAT_R8G8_B8G8_UNORM;
  case DataFormat::DATA_FORMAT_G8R8_G8B8_UNORM:
    return DXGI_FORMAT_G8R8_G8B8_UNORM;
  case DataFormat::DATA_FORMAT_R9G9B9E5_SHAREDEXP:
    return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
  case DataFormat::DATA_FORMAT_R32G32_TYPELESS:
    return DXGI_FORMAT_R32G32_TYPELESS;
  case DataFormat::DATA_FORMAT_R32G32_FLOAT:
    return DXGI_FORMAT_R32G32_FLOAT;
  case DataFormat::DATA_FORMAT_R32G32_UINT:
    return DXGI_FORMAT_R32G32_UINT;
  case DataFormat::DATA_FORMAT_R32G32_SINT:
    return DXGI_FORMAT_R32G32_SINT;
  case DataFormat::DATA_FORMAT_R32G8X24_TYPELESS:
    return DXGI_FORMAT_R32G8X24_TYPELESS;
  case DataFormat::DATA_FORMAT_D32_FLOAT_S8X24_UINT:
    return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
  case DataFormat::DATA_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
  case DataFormat::DATA_FORMAT_X32_TYPELESS_G8X24_UINT:
    return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
  case DataFormat::DATA_FORMAT_R16G16B16A16_TYPELESS:
    return DXGI_FORMAT_R16G16B16A16_TYPELESS;
  case DataFormat::DATA_FORMAT_R16G16B16A16_FLOAT:
    return DXGI_FORMAT_R16G16B16A16_FLOAT;
  case DataFormat::DATA_FORMAT_R16G16B16A16_UNORM:
    return DXGI_FORMAT_R16G16B16A16_UNORM;
  case DataFormat::DATA_FORMAT_R16G16B16A16_UINT:
    return DXGI_FORMAT_R16G16B16A16_UINT;
  case DataFormat::DATA_FORMAT_R16G16B16A16_SNORM:
    return DXGI_FORMAT_R16G16B16A16_SNORM;
  case DataFormat::DATA_FORMAT_R16G16B16A16_SINT:
    return DXGI_FORMAT_R16G16B16A16_SINT;
  case DataFormat::DATA_FORMAT_R32G32B32_TYPELESS:
    return DXGI_FORMAT_R32G32B32_TYPELESS;
  case DataFormat::DATA_FORMAT_R32G32B32_FLOAT:
    return DXGI_FORMAT_R32G32B32_FLOAT;
  case DataFormat::DATA_FORMAT_R32G32B32_UINT:
    return DXGI_FORMAT_R32G32B32_UINT;
  case DataFormat::DATA_FORMAT_R32G32B32_SINT:
    return DXGI_FORMAT_R32G32B32_SINT;
  case DataFormat::DATA_FORMAT_R32G32B32A32_TYPELESS:
    return DXGI_FORMAT_R32G32B32A32_TYPELESS;
  case DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT:
    return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case DataFormat::DATA_FORMAT_R32G32B32A32_UINT:
    return DXGI_FORMAT_R32G32B32A32_UINT;
  case DataFormat::DATA_FORMAT_R32G32B32A32_SINT:
    return DXGI_FORMAT_R32G32B32A32_SINT;
  }
  return DXGI_FORMAT_UNKNOWN;
}
BindType convertToBindType(_D3D_SHADER_INPUT_TYPE type) {

  switch (type) {
  case D3D_SIT_CBUFFER:
    return BindType::BIND_TYPE_SIT_CBUFFER;
  case D3D_SIT_TBUFFER:
    return BindType::BIND_TYPE_SIT_TBUFFER;
  case D3D_SIT_TEXTURE:
    return BindType::BIND_TYPE_SIT_TEXTURE;
  case D3D_SIT_SAMPLER:
    return BindType::BIND_TYPE_SIT_SAMPLER;
  case D3D_SIT_UAV_RWTYPED:
    return BindType::BIND_TYPE_SIT_UAV_RWTYPED;
  case D3D_SIT_STRUCTURED:
    return BindType::BIND_TYPE_SIT_STRUCTURED;
  case D3D_SIT_UAV_RWSTRUCTURED:
    return BindType::BIND_TYPE_SIT_UAV_RWSTRUCTURED;
  case D3D_SIT_BYTEADDRESS:
    return BindType::BIND_TYPE_SIT_BYTEADDRESS;
  case D3D_SIT_UAV_RWBYTEADDRESS:
    return BindType::BIND_TYPE_SIT_UAV_RWBYTEADDRESS;
  case D3D_SIT_UAV_APPEND_STRUCTURED:
    return BindType::BIND_TYPE_SIT_UAV_APPEND_STRUCTURED;
  case D3D_SIT_UAV_CONSUME_STRUCTURED:
    return BindType::BIND_TYPE_SIT_UAV_CONSUME_STRUCTURED;
  case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
    return BindType::BIND_TYPE_SIT_UAV_RWSTRUCTURED_WITH_COUNTER;
  case D3D_SIT_RTACCELERATIONSTRUCTURE:
    return BindType::BIND_TYPE_SIT_RTACCELERATIONSTRUCTURE;
  case D3D_SIT_UAV_FEEDBACKTEXTURE:
    return BindType::BIND_TYPE_SIT_UAV_FEEDBACKTEXTURE;
  }
  return BindType::BIND_TYPE_SIT_CBUFFER;
}
DataDimension convertToDataDimenstion(D3D_SRV_DIMENSION dimension) {
  switch (dimension) {
  case D3D_SRV_DIMENSION_UNKNOWN:
    return DataDimension::DATA_DIMENSION_UNKNOWN;
  case D3D_SRV_DIMENSION_BUFFER:
    return DataDimension::DATA_DIMENSION_BUFFER;
  case D3D_SRV_DIMENSION_TEXTURE1D:
    return DataDimension::DATA_DIMENSION_TEXTURE1D;
  case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
    return DataDimension::DATA_DIMENSION_TEXTURE1DARRAY;
  case D3D_SRV_DIMENSION_TEXTURE2D:
    return DataDimension::DATA_DIMENSION_TEXTURE2D;
  case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
    return DataDimension::DATA_DIMENSION_TEXTURE2DARRAY;
  case D3D_SRV_DIMENSION_TEXTURE2DMS:
    return DataDimension::DATA_DIMENSION_TEXTURE2DMS;
  case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
    return DataDimension::DATA_DIMENSION_TEXTURE2DMSARRAY;
  case D3D_SRV_DIMENSION_TEXTURE3D:
    return DataDimension::DATA_DIMENSION_TEXTURE3D;
  case D3D_SRV_DIMENSION_TEXTURECUBE:
    return DataDimension::DATA_DIMENSION_TEXTURECUBE;
  case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
    return DataDimension::DATA_DIMENSION_TEXTURECUBEARRAY;
  case D3D_SRV_DIMENSION_BUFFEREX:
    return DataDimension::DATA_DIMENSION_BUFFEREX;
  }
  return DataDimension::DATA_DIMENSION_UNKNOWN;
}
D3D12_SHADER_VISIBILITY convertToD3D12ShaderVisibility(ShaderType type) {

  switch (type) {
  case ShaderType::SHADER_TYPE_VERTEX:
    return D3D12_SHADER_VISIBILITY_VERTEX;
  case ShaderType::SHADER_TYPE_PIXEL:
    return D3D12_SHADER_VISIBILITY_PIXEL;
  case ShaderType::SHADER_TYPE_GEOMETRY:
    return D3D12_SHADER_VISIBILITY_GEOMETRY;
  case ShaderType::SHADER_TYPE_HULL:
    return D3D12_SHADER_VISIBILITY_HULL;
  case ShaderType::SHADER_TYPE_DOMAIN:
    return D3D12_SHADER_VISIBILITY_DOMAIN;
  case ShaderType::SHADER_TYPE_AMPLIFICATION:
    return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
  case ShaderType::SHADER_TYPE_MESH:
    return D3D12_SHADER_VISIBILITY_MESH;
  }
  // unknown, all  and compute here
  return D3D12_SHADER_VISIBILITY_ALL;
}
D3D12_ROOT_PARAMETER_TYPE convertToD3D12RootParameterType(BindType type) {
  if (type == BindType::BIND_TYPE_CBV_BOUND ||
      type == BindType::BIND_TYPE_SRV_BOUND ||
      type == BindType::BIND_TYPE_UAV_BOUND ||
      type == BindType::BIND_TYPE_SIT_SAMPLER ||
      type == BindType::BIND_TYPE_SIT_RTACCELERATIONSTRUCTURE) {
    throw std::exception(
        (std::string(
             "Error bind type convert to D3D12_ROOT_PARAMETER_TYPE : ") +
         std::string(magic_enum::enum_name(type)))
            .c_str());
  }
  if (type < BindType::BIND_TYPE_CBV_BOUND)
    return D3D12_ROOT_PARAMETER_TYPE_CBV;
  if (type < BindType::BIND_TYPE_SRV_BOUND)
    return D3D12_ROOT_PARAMETER_TYPE_SRV;
  if (type < BindType::BIND_TYPE_UAV_BOUND)
    return D3D12_ROOT_PARAMETER_TYPE_UAV;
  return D3D12_ROOT_PARAMETER_TYPE_CBV;
}
D3D12_DESCRIPTOR_RANGE_TYPE convertToD3D12DescriptorRangeType(BindType type) {
  if (type == BindType::BIND_TYPE_CBV_BOUND ||
      type == BindType::BIND_TYPE_SRV_BOUND ||
      type == BindType::BIND_TYPE_UAV_BOUND ||
      type == BindType::BIND_TYPE_SIT_RTACCELERATIONSTRUCTURE) {
    throw std::exception(
        (std::string(
             "Error bind type convert to D3D12_DISCRIPTOR_RANGE_TYPE : ") +
         std::string(magic_enum::enum_name(type)))
            .c_str());
  }
  if (type == BindType::BIND_TYPE_SIT_SAMPLER)
    return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  if (type < BindType::BIND_TYPE_CBV_BOUND)
    return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
  if (type < BindType::BIND_TYPE_SRV_BOUND)
    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  if (type < BindType::BIND_TYPE_UAV_BOUND)
    return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
  return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
}
D3D12_BLEND convertToD3D12Blend(Blend blend) {
  switch (blend) {
  case Blend::BLEND_ZERO:
    return D3D12_BLEND_ZERO;
  case Blend::BLEND_ONE:
    return D3D12_BLEND_ONE;
  case Blend::BLEND_SRC_COLOR:
    return D3D12_BLEND_SRC_COLOR;
  case Blend::BLEND_INV_SRC_COLOR:
    return D3D12_BLEND_INV_SRC_COLOR;
  case Blend::BLEND_SRC_ALPHA:
    return D3D12_BLEND_SRC_ALPHA;
  case Blend::BLEND_INV_SRC_ALPHA:
    return D3D12_BLEND_INV_SRC_ALPHA;
  case Blend::BLEND_DEST_ALPHA:
    return D3D12_BLEND_DEST_ALPHA;
  case Blend::BLEND_INV_DEST_ALPHA:
    return D3D12_BLEND_INV_DEST_ALPHA;
  case Blend::BLEND_DEST_COLOR:
    return D3D12_BLEND_DEST_COLOR;
  case Blend::BLEND_INV_DEST_COLOR:
    return D3D12_BLEND_INV_DEST_COLOR;
  case Blend::BLEND_SRC_ALPHA_SAT:
    return D3D12_BLEND_SRC_ALPHA_SAT;
  case Blend::BLEND_BLEND_FACTOR:
    return D3D12_BLEND_BLEND_FACTOR;
  case Blend::BLEND_INV_BLEND_FACTOR:
    return D3D12_BLEND_INV_BLEND_FACTOR;
  case Blend::BLEND_SRC1_COLOR:
    return D3D12_BLEND_SRC1_COLOR;
  case Blend::BLEND_INV_SRC1_COLOR:
    return D3D12_BLEND_INV_SRC1_COLOR;
  case Blend::BLEND_SRC1_ALPHA:
    return D3D12_BLEND_SRC1_ALPHA;
  case Blend::BLEND_INV_SRC1_ALPHA:
    return D3D12_BLEND_INV_SRC1_ALPHA;
  }
  return D3D12_BLEND_ZERO;
}
D3D12_BLEND_OP conertToD3D12BlendOP(BlendOperation operation) {
  switch (operation) {
  case BlendOperation::BLEND_OPERATION_ADD:
    return D3D12_BLEND_OP_ADD;
  case BlendOperation::BLEND_OPERATION_SUBTRACT:
    return D3D12_BLEND_OP_SUBTRACT;
  case BlendOperation::BLEND_OPERATION_REV_SUBTRACT:
    return D3D12_BLEND_OP_REV_SUBTRACT;
  case BlendOperation::BLEND_OPERATION_MIN:
    return D3D12_BLEND_OP_MIN;
  case BlendOperation::BLEND_OPERATION_MAX:
    return D3D12_BLEND_OP_MAX;
  }
  return D3D12_BLEND_OP_ADD;
}
D3D12_LOGIC_OP convertToD3D12LogicOP(LogicOperation logic_operation) {
  switch (logic_operation) {
  case LogicOperation::LOGIC_OPERATION_CLEAR:
    return D3D12_LOGIC_OP_CLEAR;
  case LogicOperation::LOGIC_OPERATION_SET:
    return D3D12_LOGIC_OP_SET;
  case LogicOperation::LOGIC_OPERATION_COPY:
    return D3D12_LOGIC_OP_COPY;
  case LogicOperation::LOGIC_OPERATION_COPY_INVERTED:
    return D3D12_LOGIC_OP_COPY_INVERTED;
  case LogicOperation::LOGIC_OPERATION_NOOP:
    return D3D12_LOGIC_OP_NOOP;
  case LogicOperation::LOGIC_OPERATION_INVERT:
    return D3D12_LOGIC_OP_INVERT;
  case LogicOperation::LOGIC_OPERATION_AND:
    return D3D12_LOGIC_OP_AND;
  case LogicOperation::LOGIC_OPERATION_NAND:
    return D3D12_LOGIC_OP_NAND;
  case LogicOperation::LOGIC_OPERATION_OR:
    return D3D12_LOGIC_OP_OR;
  case LogicOperation::LOGIC_OPERATION_NOR:
    return D3D12_LOGIC_OP_NOR;
  case LogicOperation::LOGIC_OPERATION_XOR:
    return D3D12_LOGIC_OP_XOR;
  case LogicOperation::LOGIC_OPERATION_EQUIV:
    return D3D12_LOGIC_OP_EQUIV;
  case LogicOperation::LOGIC_OPERATION_AND_REVERSE:
    return D3D12_LOGIC_OP_AND_REVERSE;
  case LogicOperation::LOGIC_OPERATION_AND_INVERTED:
    return D3D12_LOGIC_OP_AND_INVERTED;
  case LogicOperation::LOGIC_OPERATION_OR_REVERSE:
    return D3D12_LOGIC_OP_OR_REVERSE;
  case LogicOperation::LOGIC_OPERATION_OR_INVERTED:
    return D3D12_LOGIC_OP_OR_INVERTED;
  }
  return D3D12_LOGIC_OP_CLEAR;
}
D3D12_COLOR_WRITE_ENABLE convertToD3D12ColorWriteEnbale(ColorWriteMask mask) {
  return static_cast<D3D12_COLOR_WRITE_ENABLE>(
      static_cast<std::underlying_type<ColorWriteMask>::type>(mask));
}
D3D12_DEPTH_WRITE_MASK convertToD3D12DepthWriteMask(DepthWriteMask mask) {
  return static_cast<D3D12_DEPTH_WRITE_MASK>(
      static_cast<std::underlying_type<DepthWriteMask>::type>(mask));
}
D3D12_COMPARISON_FUNC
convertToD3D12ComparisonFUNC(ComparisonFunction function) {
  switch (function) {
  case ComparisonFunction::COMPARISON_FUNCTION_NEVER:
    return D3D12_COMPARISON_FUNC_NEVER;
  case ComparisonFunction::COMPARISON_FUNCTION_LESS:
    return D3D12_COMPARISON_FUNC_LESS;
  case ComparisonFunction::COMPARISON_FUNCTION_EQUAL:
    return D3D12_COMPARISON_FUNC_EQUAL;
  case ComparisonFunction::COMPARISON_FUNCTION_LESS_EQUAL:
    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
  case ComparisonFunction::COMPARISON_FUNCTION_GREATER:
    return D3D12_COMPARISON_FUNC_GREATER;
  case ComparisonFunction::COMPARISON_FUNCTION_NOT_EQUAL:
    return D3D12_COMPARISON_FUNC_NOT_EQUAL;
  case ComparisonFunction::COMPARISON_FUNCTION_GREATER_EQUAL:
    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
  case ComparisonFunction::COMPARISON_FUNCTION_ALWAYS:
    return D3D12_COMPARISON_FUNC_ALWAYS;
  }
  return D3D12_COMPARISON_FUNC_ALWAYS;
}
D3D12_STENCIL_OP convertToD3D12StencilOP(StencilOperation stencil_operation) {
  switch (stencil_operation) {
  case StencilOperation::STENCIL_OPERATION_KEEP:
    return D3D12_STENCIL_OP_KEEP;
  case StencilOperation::STENCIL_OPERATION_ZERO:
    return D3D12_STENCIL_OP_ZERO;
  case StencilOperation::STENCIL_OPERATION_REPLACE:
    return D3D12_STENCIL_OP_REPLACE;
  case StencilOperation::STENCIL_OPERATION_INCR_SAT:
    return D3D12_STENCIL_OP_INCR_SAT;
  case StencilOperation::STENCIL_OPERATION_DECR_SAT:
    return D3D12_STENCIL_OP_DECR_SAT;
  case StencilOperation::STENCIL_OPERATION_INVERT:
    return D3D12_STENCIL_OP_INVERT;
  case StencilOperation::STENCIL_OPERATION_INCR:
    return D3D12_STENCIL_OP_INCR;
  case StencilOperation::STENCIL_OPERATION_DECR:
    return D3D12_STENCIL_OP_DECR;
  }
  return D3D12_STENCIL_OP_KEEP;
}
D3D12_FILL_MODE convertToD3D12FillMode(FillMode fill_mode) {
  switch (fill_mode) {
  case FillMode::FILL_MODE_SOLID:
    return D3D12_FILL_MODE_SOLID;
  case FillMode::FILL_MODE_WIREFRAME:
    return D3D12_FILL_MODE_WIREFRAME;
  }
  return D3D12_FILL_MODE_SOLID;
}
D3D12_CULL_MODE convertToD3D12CullMode(CullMode cull_mode) {
  switch (cull_mode) {
  case CullMode::CULL_MODE_NONE:
    return D3D12_CULL_MODE_NONE;
  case CullMode::CULL_MODE_FRONT:
    return D3D12_CULL_MODE_FRONT;
  case CullMode::CULL_MODE_BACK:
    return D3D12_CULL_MODE_BACK;
  } // namespace Renderer
  return D3D12_CULL_MODE_NONE;
} // namespace CHCEngine

D3D12_PRIMITIVE_TOPOLOGY_TYPE
convertToD3D12PrimitiveTopologyType(PrimitiveTopologyType type) {
  switch (type) {
  case PrimitiveTopologyType::PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
  case PrimitiveTopologyType::PRIMITIVE_TOPOLOGY_TYPE_POINT:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
  case PrimitiveTopologyType::PRIMITIVE_TOPOLOGY_TYPE_LINE:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
  case PrimitiveTopologyType::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  case PrimitiveTopologyType::PRIMITIVE_TOPOLOGY_TYPE_PATCH:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
  }
  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}
D3D_PRIMITIVE_TOPOLOGY
convertToD3DPrimitiveTopology(PrimitiveTopology topology) {
  switch (topology) {
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_UNDEFINED:
    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_POINTLIST:
    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_LINELIST:
    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_LINESTRIP:
    return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST:
    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
    return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
    return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
  case PrimitiveTopology::PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST:
    return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
  }
  return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}
D3D12_DSV_FLAGS convertToD3D12DSVFlags(DepthStencilFlags flag) {
  if (flag == DepthStencilFlags::DEPTH_SENCIL_FLAG_READ_ONLY) {
    return D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
  }
  return D3D12_DSV_FLAGS(static_cast<unsigned int>(flag));
}
D3D12_RESOURCE_DIMENSION convertToD3D12ResourceDimension(TextureType type) {
  switch (type) {
  case TextureType::TEXTURE_TYPE_1D:
    return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
  case TextureType::TEXTURE_TYPE_2D:
    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  case TextureType::TEXTURE_TYPE_3D:
    return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
  }
  return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
}
D3D12_FILTER convertToD3D12Filter(Filter filter) {
  return D3D12_FILTER(static_cast<unsigned int>(filter));
}
D3D12_TEXTURE_ADDRESS_MODE
converToD3D12TextureAddressMode(TextureAddressMode mode) {
  return D3D12_TEXTURE_ADDRESS_MODE(static_cast<unsigned int>(mode));
}
} // namespace Renderer
} // namespace CHCEngine
