#include "../ClassName.h"

#include "Shader.h"

#include <iostream>
#include <magic_enum.hpp>

#include "../D3D12Convert.h"
#include "../D3D12Utilities.hpp"
#include <unordered_set>

namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
const static std::unordered_map<ShaderType, const char *> targetchars{
    {ShaderType::SHADER_TYPE_VERTEX, "vs_5_1"},
    {ShaderType::SHADER_TYPE_PIXEL, "ps_5_1"},
    {ShaderType::SHADER_TYPE_COMPUTE, "cs_5_1"},
    {ShaderType::SHADER_TYPE_GEOMETRY, "gs_5_1"},
    {ShaderType::SHADER_TYPE_DOMAIN, "ds_5_1"},
    {ShaderType::SHADER_TYPE_HULL, "hs_5_1"},
    {ShaderType::SHADER_TYPE_AMPLIFICATION,
     "as_6_5"}, // this is not going to use unless I got into mesh shader,so it
                // might go wrong
    {ShaderType::SHADER_TYPE_MESH, "ms_6_5"}};

Shader::Shader(std::string const &code_or_file, std::string const &entry_point,
               ShaderType type, bool from_file,
               const std::vector<std::string> &compute_root_constant_names)
    : entry_point_(entry_point), type_(type) {
  ComPtr<Blob> errors;
  if (compute_root_constant_names.size() &&
      type != ShaderType::SHADER_TYPE_COMPUTE) {
    throw std::exception(
        "Only compute shader can set root constant while loading shader!");
  }
#if defined(_DEBUG)
  // Enable better shader debugging with the graphics debugging tools.
  UINT compileFlags = D3DCOMPILE_DEBUG |
                      D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES |
                      D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#else
  UINT compileFlags = D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES |
                      D3DCOMPILE_SKIP_OPTIMIZATION |
                      D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#endif
  if (!from_file) {

    auto hr = D3DCompile2(code_or_file.c_str(), code_or_file.size(), NULL, NULL,
                          D3D_COMPILE_STANDARD_FILE_INCLUDE,
                          entry_point.c_str(), targetchars.find(type)->second,
                          compileFlags, 0, 0, NULL, 0, &byte_code_, &errors);
    if (errors != nullptr) {
      OutputDebugStringA((char *)errors->GetBufferPointer());
    }
    ThrowIfFailed(hr);
  } else {
    std::wstring strins;
    strins.resize(256);
    WCHAR wsz[256];
    swprintf(wsz, 256, L"%S", code_or_file.c_str());
    HRESULT hr = D3DCompileFromFile(
        wsz, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point.c_str(),
        targetchars.find(type)->second, compileFlags, 0, &byte_code_, &errors);
    if (errors != nullptr) {
      OutputDebugStringA((char *)errors->GetBufferPointer());
    }
    ThrowIfFailed(hr);
  }
  ThrowIfFailed(D3DReflect(byte_code_->GetBufferPointer(),
                           byte_code_->GetBufferSize(),
                           IID_PPV_ARGS(&shader_reflection_)));

  std::string name;
  unsigned int index = 0;
  std::unordered_set<std::string> root_names_(
      compute_root_constant_names.begin(), compute_root_constant_names.end());
  while (true) {
    D3D12_SHADER_INPUT_BIND_DESC inputdesc;
    inputdesc.Name = "";
    shader_reflection_->GetResourceBindingDesc(index, &inputdesc);
    std::string temp = inputdesc.Name;
    if (temp != "" && name != temp) {
      resource_bind_table_[temp] = {
          temp,
          convertToBindType(inputdesc.Type),
          convertToDataDimenstion(inputdesc.Dimension),
          getUsage(convertToBindType(inputdesc.Type)),
          type_,
          inputdesc.BindCount,
          inputdesc.BindPoint,
          inputdesc.Space,
      };
      if (root_names_.count(temp)) {
        auto size = getConstantBufferSizeByName(temp);
        if (size != 0) {
          resource_bind_table_[temp].type_ =
              BindType::BIND_TYPE_SIT_ROOT_CONSTANT;
          resource_bind_table_[temp].resource_count_ = size / 4;
          root_names_.erase(temp);
        }
      }
    } else {
      break;
    }
    ++index;
    name = temp;
  }

  D3D12_SHADER_DESC desc;
  shader_reflection_->GetDesc(&desc);
  D3D12_SIGNATURE_PARAMETER_DESC input_desc;
  // std::unordered_map<std::string, InputFormat> table;
  for (unsigned int i = 0; i < desc.InputParameters; i++) {
    InputFormat format;
    shader_reflection_->GetInputParameterDesc(i, &input_desc);
    std::string temp(input_desc.SemanticName);
    // system sematics don't care;
    if (temp[0] == 'S' && temp[1] == 'V' && temp[2] == '_')
      continue;
    switch (input_desc.Mask) {
    case 1:
      switch (input_desc.ComponentType) {
      case D3D_REGISTER_COMPONENT_UINT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32_UINT;
        break;
      case D3D_REGISTER_COMPONENT_SINT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32_SINT;
        break;
      case D3D_REGISTER_COMPONENT_FLOAT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32_FLOAT;
        break;
      default:
        format.data_format_ = DataFormat::DATA_FORMAT_UNKNOWN;
      }
      break;
    case 3:
      switch (input_desc.ComponentType) {
      case D3D_REGISTER_COMPONENT_UINT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32_UINT;
        break;
      case D3D_REGISTER_COMPONENT_SINT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32_SINT;
        break;
      case D3D_REGISTER_COMPONENT_FLOAT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32_FLOAT;
        break;
      default:
        format.data_format_ = DataFormat::DATA_FORMAT_UNKNOWN;
      }
      break;
    case 7:
      switch (input_desc.ComponentType) {
      case D3D_REGISTER_COMPONENT_UINT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32B32_UINT;
        break;
      case D3D_REGISTER_COMPONENT_SINT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32B32_SINT;
        break;
      case D3D_REGISTER_COMPONENT_FLOAT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32B32_FLOAT;
        break;
      default:
        format.data_format_ = DataFormat::DATA_FORMAT_UNKNOWN;
      }
      break;
    case 15:
      switch (input_desc.ComponentType) {
      case D3D_REGISTER_COMPONENT_UINT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_UINT;
        break;
      case D3D_REGISTER_COMPONENT_SINT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_SINT;
        break;
      case D3D_REGISTER_COMPONENT_FLOAT32:
        format.data_format_ = DataFormat::DATA_FORMAT_R32G32B32A32_FLOAT;
        break;
      default:
        format.data_format_ = DataFormat::DATA_FORMAT_UNKNOWN;
      }
      break;
    }
    format.sematic_index_ = input_desc.SemanticIndex;
    input_table_[temp] = format;
  }
}
std::vector<BindFormat> Shader::getBindFormats(BindType types) const {
  std::vector<BindFormat> res;
  for (auto &format : resource_bind_table_) {
    if (static_cast<unsigned int>(format.second.type_ & types) > 0) {
      res.emplace_back(format.second);
    }
  }
  return res;
}
std::vector<BindFormat> Shader::getBindFormatsExclude(BindType types) const {
  std::vector<BindFormat> res;
  for (auto &format : resource_bind_table_) {
    if (static_cast<unsigned int>(format.second.type_ & types) == 0) {
      res.emplace_back(format.second);
    }
  }
  return res;
}
bool Shader::hasFormat(const std::string &name) {
  return resource_bind_table_.count(name);
}
// size always has alignment with 32bytes
uint32_t Shader::getConstantBufferSizeByName(const std::string &name) const {
  if (!resource_bind_table_.count(name))
    return 0;
  if (resource_bind_table_.at(name).type_ != BindType::BIND_TYPE_SIT_CBUFFER)
    return 0;
  auto const_buffer = shader_reflection_->GetConstantBufferByName(name.c_str());

  // probably can have more specific binding in future

  D3D12_SHADER_BUFFER_DESC desc;
  const_buffer->GetDesc(&desc);
  /*for (uint32_t i = 0; i < desc.Variables; ++i) {
    D3D12_SHADER_VARIABLE_DESC var_desc;
    auto var = const_buffer->GetVariableByIndex(i);
    var->GetDesc(&var_desc);

  }*/
  return desc.Size;
}
const std::unordered_map<std::string, InputFormat> &
Shader::getInputTable() const {
  return input_table_;
}
std::vector<OutputFormat> Shader::getOutputTable() const {
  D3D12_SHADER_DESC desc;
  shader_reflection_->GetDesc(&desc);
  D3D12_SIGNATURE_PARAMETER_DESC output_desc;
  std::vector<OutputFormat> table;
  for (unsigned int i = 0; i < desc.OutputParameters; i++) {
    shader_reflection_->GetOutputParameterDesc(i, &output_desc);
    OutputFormat format;
    switch (output_desc.Mask) {
    case 1:
      format.channel_count_ = 1;
      break;
    case 3:
      format.channel_count_ = 2;
      break;
    case 7:
      format.channel_count_ = 3;
      break;
    case 15:
      format.channel_count_ = 4;
      break;
    }
    table.push_back(format);
  }

  return table;
}
void ShaderSet::updateResourceBindTable() {
  resource_bind_table_.clear();
  uint32_t constant_count = 0;
  std::unordered_set<std::string> temp(root_constant_names_);
  for (auto &shader : shader_set_) {
    for (auto &format : shader.second.getBindTable()) {
      // didn't appear we add it
      if (!resource_bind_table_.count(format.first)) {
        resource_bind_table_.emplace(format.first, format.second);
        if (temp.count(format.first)) {
          auto size = shader.second.getConstantBufferSizeByName(format.first);
          if (size != 0) {
            resource_bind_table_[format.first].type_ =
                BindType::BIND_TYPE_SIT_ROOT_CONSTANT;
            resource_bind_table_[format.first].resource_count_ = size / 4;
            temp.erase(format.first);
            ++constant_count;
          }
        }
      } else {
        resource_bind_table_[format.first].visiblity_ =
            ShaderType::SHADER_TYPE_ALL;
      }
    }
  }
  if (temp.size()) {
    std::string error("Have Invalid root constant name :");
    for (auto &s : temp) {
      error += " , ";
      error += s;
    }
    throw std::exception(error.c_str());
  }
}
ShaderSet::ShaderSet(const std::unordered_map<ShaderType, Shader> &shader_set)
    : shader_set_(shader_set) {
  if (shader_set_.count(ShaderType::SHADER_TYPE_COMPUTE)) {
    throw std::exception("Compute Shader can't add to shader set");
  }
  updateResourceBindTable();
}
ShaderSet::ShaderSet(const std::vector<Shader> &shader_set,
                     const std::vector<std::string> &root_constants_names)
    : root_constant_names_(root_constants_names.begin(),
                           root_constants_names.end()) {
  addAsVector(shader_set);
}
ShaderSet::ShaderSet(const std::unordered_map<ShaderType, Shader> &shader_set,
                     const std::vector<std::string> &root_constants_names)
    : root_constant_names_(root_constants_names.begin(),
                           root_constants_names.end()) {
  if (shader_set_.count(ShaderType::SHADER_TYPE_COMPUTE)) {
    throw std::exception("Compute Shader can't add to shader set");
  }
  updateResourceBindTable();
}
void ShaderSet::addAsVector(const std::vector<Shader> &shaders) {
  for (auto &shader : shaders) {
    if (shader_set_.count(shader.getType())) {
      throw std::exception(
          std::string("Add same type shader to shader set:")
              .append(std::string(magic_enum::enum_name(shader.getType())))
              .c_str());
    }
    shader_set_.emplace(shader.getType(), shader);
  }
  if (shader_set_.count(ShaderType::SHADER_TYPE_COMPUTE)) {
    throw std::exception("Compute Shader can't add to shader set");
  }
  updateResourceBindTable();
}
bool ShaderSet::hasShader(ShaderType type) const {
  return shader_set_.count(type) != 0;
}
const Shader &ShaderSet::getShader(ShaderType type) const {
  if (!hasShader(type)) {
    throw std::exception(std::string("In shader set,Can't find shader type :")
                             .append(std::string(magic_enum::enum_name(type)))
                             .c_str());
  }
  return shader_set_.at(type);
}
bool ShaderSet::hasFormat(const std::string &name) {
  return resource_bind_table_.count(name);
}
std::vector<BindFormat> ShaderSet::getBindFormats(BindType types) {
  std::vector<BindFormat> res;
  for (auto &format : resource_bind_table_) {
    if (static_cast<unsigned int>(format.second.type_ & types) > 0) {
      res.emplace_back(format.second);
    }
  }
  return res;
}
std::vector<BindFormat> ShaderSet::getBindFormatsExclude(BindType types) {
  std::vector<BindFormat> res;
  for (auto &format : resource_bind_table_) {
    if (static_cast<unsigned int>(format.second.type_ & types) == 0) {
      res.emplace_back(format.second);
    }
  }
  return res;
}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine