#pragma once
#include "ClassName.h"
#include "Pipeline/BindLayout.h"
#include "Pipeline/Pipeline.h"
#include "Sampler/Sampler.h"

namespace CHCEngine {
namespace Renderer {

using Pipeline::BindLayout;
using Pipeline::BlendState;
using Pipeline::DepthStencilSetup;
using Pipeline::Rasterizer;
using Pipeline::RenderTargetSetup;
using Pipeline::ShaderSet;
// return the dword of root build by this, should be no over than 64
// tale : 1
// root constant : 1
// root descritpr : 2
// cbv with one in slot is always root desc, sampler will use table always
// other one in slot depends on the mode ,relax mode will keep all the
// bind in root desc
// return the <relax,pack> size, if pack is still over 64, it should a exception
static const BindType unbuffer =
    BindType::BIND_TYPE_SIT_TEXTURE | BindType::BIND_TYPE_SIT_SAMPLER;

inline bool canUseDescriptor(BindType type, DataDimension dimension) {
  if (dimension != DataDimension::DATA_DIMENSION_BUFFER ||
      dimension != DataDimension::DATA_DIMENSION_BUFFEREX)
    return false;
  if (static_cast<unsigned int>(type & unbuffer)) {
    return false;
  }
  return true;
}

std::pair<unsigned int, unsigned int>
calRootSignatureSize(const std::vector<Pipeline::BindSlot> &bind_layout) {
  unsigned int relax = 0;
  unsigned int pack = 0;
  for (int i = 0; i < bind_layout.size(); ++i) {
    auto &formats = bind_layout[i].formats_;
    if (formats[0].type_ == BindType::BIND_TYPE_SIT_ROOT_CONSTANT) {
      ++relax;
      ++pack;
      // just goint to table for sampler
    } else if (formats[0].type_ == BindType::BIND_TYPE_SIT_SAMPLER) {
      ++relax;
      ++pack;
    } else {
      if (formats.size() == 1 &&
          formats[0].resource_count_ == 1) { // singal desc case
        if (formats[0].type_ == BindType::BIND_TYPE_SIT_CBUFFER) {
          relax += 2;
          pack += 2;
        } else { // only buffer can use pure disc
          if (canUseDescriptor(formats[0].type_, formats[0].dimension_)) {
            relax += 2;
            pack += 1;
          } else {
            ++relax;
            ++pack;
          }
        }
      } else {
        ++relax;
        ++pack;
      }
    }
  }
  return {relax, pack};
}
bool shouldUseTable(const Pipeline::BindSlot &slot, bool relax) {
  if (slot.formats_.size() > 1)
    return true;
  if (slot.formats_[0].type_ == BindType::BIND_TYPE_SIT_ROOT_CONSTANT)
    return false;
  if (slot.formats_[0].type_ == BindType::BIND_TYPE_SIT_SAMPLER)
    return true;
  if (slot.formats_[0].resource_count_ == 1) {
    if (slot.formats_[0].type_ == BindType::BIND_TYPE_SIT_CBUFFER ||
        (canUseDescriptor(slot.formats_[0].type_,
                          slot.formats_[0].dimension_) &&
         relax)) {
      return false;
    }
  }
  // one format with multile resource bind liek texture2d tt[4] or unboud
  // the count should be 0
  return true;
}
// very akward way, but since it's just a help function, I will keep this
void generateRootParameters(
    const std::vector<Pipeline::BindSlot> &bind_slots,
    std::vector<D3D12_ROOT_PARAMETER1> &RootParameters,
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE1>> &ranges,
    std::vector<bool> &direct_binds, bool relax) {
  RootParameters.resize(bind_slots.size());
  ranges.resize(bind_slots.size());
  for (int i = 0; i < bind_slots.size(); ++i) {
    auto &parameter = RootParameters[i];
    if (bind_slots[i].formats_[0].type_ ==
        BindType::BIND_TYPE_SIT_ROOT_CONSTANT) {
      parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
      parameter.ShaderVisibility =
          convertToD3D12ShaderVisibility(bind_slots[i].formats_[0].visiblity_);
      parameter.Constants.Num32BitValues =
          bind_slots[i].formats_[0].resource_count_;
      parameter.Constants.ShaderRegister = bind_slots[i].formats_[0].point_;
      parameter.Constants.RegisterSpace = bind_slots[i].formats_[0].space_;
      direct_binds.push_back(true);
      continue;
    }
    if (!shouldUseTable(bind_slots[i], relax)) {
      parameter.ParameterType =
          convertToD3D12RootParameterType(bind_slots[i].formats_[0].type_);
      parameter.ShaderVisibility =
          convertToD3D12ShaderVisibility(bind_slots[i].formats_[0].visiblity_);
      parameter.Descriptor.ShaderRegister = bind_slots[i].formats_[0].point_;
      parameter.Descriptor.RegisterSpace = bind_slots[i].formats_[0].space_;
      // use default setting
      parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
      direct_binds.push_back(true);
      continue;
    } else {
      // use one format as one range , probably can have more better way
      ShaderType current_type = bind_slots[i].visibility_;
      ranges[i].resize(bind_slots[i].formats_.size());
      for (unsigned int j = 0; j < bind_slots[i].formats_.size(); ++j) {
        D3D12_DESCRIPTOR_RANGE1 r = {
            convertToD3D12DescriptorRangeType(bind_slots[i].formats_[j].type_),
            bind_slots[i].formats_[j].resource_count_,
            bind_slots[i].formats_[j].point_,
            bind_slots[i].formats_[j].space_,
            D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
        if (r.NumDescriptors == 0) { // unbound case
          r.NumDescriptors = -1;
          r.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
        }
        ranges[i][j] = r;
      }
      parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
      parameter.ShaderVisibility = convertToD3D12ShaderVisibility(current_type);
      parameter.DescriptorTable.NumDescriptorRanges =
          static_cast<unsigned int>(bind_slots[i].formats_.size());
      parameter.DescriptorTable.pDescriptorRanges = ranges[i].data();
      direct_binds.push_back(false);
    }
  }
} // namespace Renderer
inline void fillShaderByteCodes(
    D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc,
    const std::unordered_map<ShaderType, ComPtr<Blob>> &byte_codes) {
  if (byte_codes.count(ShaderType::SHADER_TYPE_VERTEX)) {
    desc.VS = CD3DX12_SHADER_BYTECODE(
        byte_codes.at(ShaderType::SHADER_TYPE_VERTEX).Get());

    /*{
        byte_codes.at(ShaderType::SHADER_TYPE_VERTEX)->GetBufferPointer(),
        byte_codes.at(ShaderType::SHADER_TYPE_VERTEX)->GetBufferSize()};*/
  }
  if (byte_codes.count(ShaderType::SHADER_TYPE_PIXEL)) {
    desc.PS = CD3DX12_SHADER_BYTECODE(
        byte_codes.at(ShaderType::SHADER_TYPE_PIXEL).Get());
    /*{byte_codes.at(ShaderType::SHADER_TYPE_PIXEL)->GetBufferPointer(),
               byte_codes.at(ShaderType::SHADER_TYPE_PIXEL)->GetBufferSize()};*/
  }
  if (byte_codes.count(ShaderType::SHADER_TYPE_GEOMETRY)) {
    desc.GS = {
        byte_codes.at(ShaderType::SHADER_TYPE_GEOMETRY)->GetBufferPointer(),
        byte_codes.at(ShaderType::SHADER_TYPE_GEOMETRY)->GetBufferSize()};
  }
  if (byte_codes.count(ShaderType::SHADER_TYPE_HULL)) {
    desc.HS = {byte_codes.at(ShaderType::SHADER_TYPE_HULL)->GetBufferPointer(),
               byte_codes.at(ShaderType::SHADER_TYPE_HULL)->GetBufferSize()};
  }
  if (byte_codes.count(ShaderType::SHADER_TYPE_DOMAIN)) {
    desc.DS = {
        byte_codes.at(ShaderType::SHADER_TYPE_DOMAIN)->GetBufferPointer(),
        byte_codes.at(ShaderType::SHADER_TYPE_DOMAIN)->GetBufferSize()};
  }
}

inline void
fillInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputs,
                const ShaderSet &shader_set,
                const std::vector<Resource::Attributes> &attributes) {
  // No input needed, probally gonna use in draw a large triangle for screen
  if (attributes.size() == 0)
    return;
  auto dummy = attributes[0].begin();
  std::unordered_map<std::string, std::pair<unsigned int, decltype(dummy)>>
      table;
  // merge all the input
  for (int i = 0; i < attributes.size(); ++i) {
    for (auto it = attributes[i].begin(); it != attributes[i].end(); ++it) {
      if (table.count(it->first)) {
        throw std::exception(
            (std::string("Duplicate Sematic in Input Attributes,currently not "
                         "support same sematic vertix input : ") +
             it->first)
                .c_str());
      }
      table[it->first] = {i, it};
    }
  }
  auto &shader_inputs =
      shader_set.getShader(ShaderType::SHADER_TYPE_VERTEX).getInputTable();
  inputs.resize(shader_inputs.size());
  unsigned int index = 0;
  for (auto &shader_input : shader_inputs) {
    if (table.count(shader_input.first) == 0) {
      throw std::exception(
          (std::string("Can't find sematic in input attributes : ") +
           shader_input.first)
              .c_str());
    }
    auto &attribute = table[shader_input.first];

    if (attribute.second->second.second != shader_input.second.data_format_) {
      throw std::exception((std::string("Dataformat miss match between input "
                                        "attribute and vertex attribute : ") +
                            shader_input.first)
                               .c_str());
    }

    // the only place I am going to use for const_cast
    inputs[index].SemanticName = attribute.second->first.c_str();
    inputs[index].SemanticIndex = shader_input.second.sematic_index_;
    inputs[index].Format =
        convertToDXGIFormat(shader_input.second.data_format_);
    inputs[index].InputSlot = attribute.first;
    inputs[index].AlignedByteOffset = attribute.second->second.first;
    inputs[index].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    inputs[index].InstanceDataStepRate = 0;
    ++index;
  }
}
void fillBlendState(D3D12_BLEND_DESC &blend_desc,
                    const RenderTargetSetup &render_setup) {
  blend_desc.AlphaToCoverageEnable = render_setup.isAlphaCoverage();
  blend_desc.IndependentBlendEnable = render_setup.isIndependentBlend();
  auto blends = render_setup.getBlendStates();
  for (unsigned int i = 0; i < render_setup.getSize(); ++i) {
    blend_desc.RenderTarget[i].BlendEnable = blends[i].enable_;
    blend_desc.RenderTarget[i].LogicOpEnable =
        blends[i].logic_operation_enable_;
    blend_desc.RenderTarget[i].SrcBlend =
        convertToD3D12Blend(blends[i].source_blend_);
    blend_desc.RenderTarget[i].DestBlend =
        convertToD3D12Blend(blends[i].destiny_blend_);
    blend_desc.RenderTarget[i].BlendOp =
        conertToD3D12BlendOP(blends[i].blend_operation_);
    blend_desc.RenderTarget[i].SrcBlendAlpha =
        convertToD3D12Blend(blends[i].source_alpha_blend_);
    blend_desc.RenderTarget[i].DestBlendAlpha =
        convertToD3D12Blend(blends[i].destiny_alpha_blend_);
    blend_desc.RenderTarget[i].BlendOpAlpha =
        conertToD3D12BlendOP(blends[i].blend_alpha_operation_);
    blend_desc.RenderTarget[i].LogicOp =
        convertToD3D12LogicOP(blends[i].logic_operation_);
    blend_desc.RenderTarget[i].RenderTargetWriteMask =
        static_cast<unsigned int>(blends[i].write_mask_);
  }
}
void fillDepthStencilState(D3D12_DEPTH_STENCIL_DESC &desc,
                           const DepthStencilSetup &depth) {
  desc.DepthEnable = depth.isDepthEnable();
  desc.DepthWriteMask = convertToD3D12DepthWriteMask(depth.getDepthWriteMask());
  desc.DepthFunc =
      convertToD3D12ComparisonFUNC(depth.getDepthComparisonFunction());
  desc.StencilEnable = depth.isStencilEnable();
  desc.StencilReadMask = depth.getStencilReadMask();
  desc.StencilWriteMask = depth.getStencilWriteMask();
  const auto &front = depth.getFrontFaceOpeartion();
  desc.FrontFace.StencilFailOp =
      convertToD3D12StencilOP(front.stencil_fail_operation_);
  desc.FrontFace.StencilDepthFailOp =
      convertToD3D12StencilOP(front.stencil_depth_fail_operation_);
  desc.FrontFace.StencilPassOp =
      convertToD3D12StencilOP(front.stencil_pass_operation_);
  desc.FrontFace.StencilFunc =
      convertToD3D12ComparisonFUNC(front.stencil_function_);
  const auto &back = depth.getBackFaceOpeartion();
  desc.BackFace.StencilFailOp =
      convertToD3D12StencilOP(back.stencil_fail_operation_);
  desc.BackFace.StencilDepthFailOp =
      convertToD3D12StencilOP(back.stencil_depth_fail_operation_);
  desc.BackFace.StencilPassOp =
      convertToD3D12StencilOP(back.stencil_pass_operation_);
  desc.BackFace.StencilFunc =
      convertToD3D12ComparisonFUNC(back.stencil_function_);
}
void fillRastersizer(D3D12_RASTERIZER_DESC &desc,
                     const Rasterizer &rasterizer) {
  desc.FillMode = convertToD3D12FillMode(rasterizer.fill_mode_);
  desc.CullMode = convertToD3D12CullMode(rasterizer.cull_mode_);
  desc.FrontCounterClockwise = rasterizer.front_counter_clock_wise_;
  desc.DepthBias = rasterizer.depth_bias_;
  desc.DepthBiasClamp = rasterizer.depth_bias_clamp;
  desc.SlopeScaledDepthBias = rasterizer.slope_scaled_depth_bias_;
  desc.DepthClipEnable = rasterizer.depth_clip_enable_;
  desc.MultisampleEnable = rasterizer.multi_sample_enable_;
  desc.AntialiasedLineEnable = rasterizer.anti_alaised_line_enable_;
  desc.ForcedSampleCount = rasterizer.forced_sample_count_;
  desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
  if (rasterizer.conservative_rasterization_enable_)
    desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
}
void fillSampler(D3D12_SAMPLER_DESC &sampler,
                 const Sampler::SamplerInformation &inf) {
  sampler = {};
  sampler.Filter = convertToD3D12Filter(inf.filter_);
  sampler.AddressU = converToD3D12TextureAddressMode(inf.u_mode_);
  sampler.AddressV = converToD3D12TextureAddressMode(inf.v_mode_);
  sampler.AddressW = converToD3D12TextureAddressMode(inf.w_mode_);
  sampler.BorderColor[0] = inf.border_color_.r;
  sampler.BorderColor[1] = inf.border_color_.g;
  sampler.BorderColor[2] = inf.border_color_.b;
  sampler.BorderColor[3] = inf.border_color_.a;
  sampler.ComparisonFunc = convertToD3D12ComparisonFUNC(inf.comparison_func_);
  sampler.MaxAnisotropy = inf.max_anisotropy_;
  sampler.MaxLOD = inf.max_lod;
  sampler.MinLOD = inf.min_lod;
  sampler.MipLODBias = inf.mip_lod_bias_;
}
} // namespace Renderer
} // namespace CHCEngine