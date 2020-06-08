#pragma once
#include "ClassName.h"
#include "Pipeline/BindLayout.h"

namespace CHCEngine {
namespace Renderer {

// return the dword of root build by this, should be no over than 64
// tale : 1
// root constant : 1
// root descritpr : 2
// cbv with one in slot is always root desc, sampler will use table always
// other one in slot depends on the mode ,relax mode will keep all the
// bind in root desc
// return the <relax,pack> size, if pack is still over 64, it should a exception
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
        } else {
          relax += 2;
          pack += 1;
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
    if (slot.formats_[0].type_ == BindType::BIND_TYPE_SIT_CBUFFER || relax) {
      return false;
    }
  }
  // one format with multile resource bind liek texture2d tt[4] or unboud
  // the count should be 0
  return true;
}
// vert akward way, but since it's just a help function, I will keep this
void generateRootParameters(
    const std::vector<Pipeline::BindSlot> &bind_slots,
    std::vector<D3D12_ROOT_PARAMETER1> &RootParameters,
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE1>> &ranges, bool relax) {
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
      parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
      continue;
    } else {
      // use one format as one range , probally can have more better way
      ShaderType current_type = bind_slots[i].formats_[0].visiblity_;
      ranges[i].resize(bind_slots[i].formats_.size());
      for (unsigned int j = 0; j < bind_slots[i].formats_.size(); ++j) {
        if (current_type != bind_slots[i].formats_[j].visiblity_) {
          current_type = ShaderType::SHADER_TYPE_ALL;
        }
        D3D12_DESCRIPTOR_RANGE1 r = {
            convertToD3D12DescriptorRangeType(bind_slots[i].formats_[j].type_),
            bind_slots[i].formats_[j].resource_count_,
            bind_slots[i].formats_[j].point_,
            bind_slots[i].formats_[j].space_,
            D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
        if (r.NumDescriptors==0) { // unbound case
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
    }
  }
}
} // namespace Renderer
} // namespace CHCEngine