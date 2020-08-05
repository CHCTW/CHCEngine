#pragma once
#include <wrl/client.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../ClassName.h"

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
struct InputFormat {
  unsigned int sematic_index_;
  DataFormat data_format_;
};
struct OutputFormat {
  unsigned int channel_count_;
};

struct BindFormat {
  std::string name_;
  BindType type_;
  DataDimension dimension_;
  BindUsage usage_ = BindUsage::BIND_USAGE_UNKNOWN;
  ShaderType visiblity_ = ShaderType::SHADER_TYPE_NONE;
  unsigned int resource_count_ = 1;
  unsigned int point_ = 0;
  unsigned int space_ = 0;
};

class Shader {
private:
  friend class Renderer;
  std::string entry_point_;
  ComPtr<Blob> byte_code_;
  ComPtr<ShaderReflection> shader_reflection_;
  ShaderType type_;
  std::unordered_map<std::string, BindFormat> resource_bind_table_;
  std::unordered_map<std::string, InputFormat> input_table_;
public:
  Shader(std::string const &code_or_file, std::string const &entry_point,
         ShaderType type, bool from_file = false);
  const std::unordered_map<std::string, BindFormat> &getBindTable() const {
    return resource_bind_table_;
  }
  std::vector<BindFormat> getBindFormats(BindType types) const ;
  std::vector<BindFormat> getBindFormatsExclude(BindType types) const;
  bool hasFormat(const std::string &name);
  const BindFormat &getBindFormat(std::string name) const {
    if (!resource_bind_table_.count(name))
      return BindFormat();
    return resource_bind_table_.at(name);
  }
  const std::unordered_map<std::string, InputFormat> & getInputTable() const;
  std::vector<OutputFormat> getOutputTable() const;
  ShaderType getType() const { return type_; }
};
class ShaderSet {
private:
    // probally going to modify to pointer style
  std::unordered_map<ShaderType, Shader> shader_set_;
  // with the mix result, that is shader visiblity will change
  std::unordered_map<std::string, BindFormat> resource_bind_table_;
  void updateResourceBindTable();
  void addAsVector(const std::vector<Shader> &shaders);
  friend class Renderer;
public:
  explicit ShaderSet(const std::unordered_map<ShaderType, Shader> &shader_set);
  template <class... ShaderClass>
  explicit ShaderSet(ShaderClass &&... shader_list) {
    addAsVector({std::forward<ShaderClass>(shader_list)...});
  }
  bool hasShader(ShaderType type) const;
  const Shader &getShader(ShaderType type) const;
  bool hasFormat(const std::string &name);
  const BindFormat &getBindFormat(std::string name) const {
    if (!resource_bind_table_.count(name)) {
      throw std::exception((std::string("Can't find ") + name +
                           std::string(" in Shader")).c_str());
    }
    return resource_bind_table_.at(name);
  }
  const std::unordered_map<std::string, BindFormat> &getBindingTable() const {
    return resource_bind_table_;
  }
  std::vector<BindFormat> getBindFormats(BindType types);
  std::vector<BindFormat> getBindFormatsExclude(BindType types);
};
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine