#pragma once
#include "ClassName.h"
namespace CHCEngine {
namespace Renderer {
D3D12_DESCRIPTOR_HEAP_TYPE convertToD3DDescriptorHeapType(DescriptorType type);
D3D12_COMMAND_LIST_TYPE convertToD3D12CommandType(CommandType type);
D3D12_RESOURCE_STATES convertToD3D12ResourceStates(ResourceState state);
D3D12_RESOURCE_BARRIER_FLAGS convertToD3D12ResourceBarrierFlags(
    ResourceTransitionFlag flag);
D3D12_HEAP_TYPE convertToD3D12HeapType(HeapType type);
DXGI_FORMAT convertToDXGIFormat(IndexFormat format);
DXGI_FORMAT convertToDXGIFormat(DataFormat format);
BindType convertToBindType(_D3D_SHADER_INPUT_TYPE type);
DataDimension convertToDataDimenstion(D3D_SRV_DIMENSION dimension);
D3D12_SHADER_VISIBILITY convertToD3D12ShaderVisibility(ShaderType type);
D3D12_ROOT_PARAMETER_TYPE convertToD3D12RootParameterType(BindType type);
D3D12_DESCRIPTOR_RANGE_TYPE convertToD3D12DescriptorRangeType(BindType type);
D3D12_BLEND convertToD3D12Blend(Blend blend);
D3D12_BLEND_OP conertToD3D12BlendOP(BlendOperation operation);
D3D12_LOGIC_OP convertToD3D12LogicOP(LogicOperation logic_operation);
D3D12_COLOR_WRITE_ENABLE convertToD3D12ColorWriteEnbale(ColorWriteMask mask);
D3D12_DEPTH_WRITE_MASK convertToD3D12DepthWriteMask(DepthWriteMask mask);
D3D12_COMPARISON_FUNC convertToD3D12ComparisonFUNC(ComparisonFunction function);
D3D12_STENCIL_OP convertToD3D12StencilOP(StencilOperation stencil_operation);
D3D12_FILL_MODE convertToD3D12FillMode(FillMode fill_mode);
D3D12_CULL_MODE convertToD3D12CullMode(CullMode cull_mode);
D3D12_PRIMITIVE_TOPOLOGY_TYPE
convertToD3D12PrimitiveTopologyType(PrimitiveTopologyType type);
}  // namespace Renderer
}  // namespace CHCEngine