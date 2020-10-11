#pragma once

#include <memory>
#include <mutex>
#include <string_view>
#include <vector>

#include "../Resource/Resource.h"
#include "Shader.h"

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
namespace Context {
class GraphicsContext;
class ComputeContext;
} // namespace Context
namespace Pipeline {
// blind slot will have 1~many bind format, and will have
// a name to get by bindlayout to get the index,
// the bind_name_ can set to first format name, if undeifned
// can also set a resrouce pointer for prebind
// for multiple  formats, the resrouce will be a resource group
// a resrouce group will have multiple resrouce in it

struct BindSlot {
  std::string bind_slot_name_; //
  std::vector<BindFormat> formats_;
  ShaderType visibility_;
  uint32_t bind_count_ = 0;
  bool isUnbound = false;
  // sampler can't stay in the same slot with other resource
  BindSlot() = delete;
  static bool checkAndMoveUnbound(std::vector<BindFormat> &formats);
  BindSlot(const std::string &bind_slot_name,
           const std::vector<BindFormat> &formats)
      : formats_(formats) {
    if (!checkAndMoveUnbound(formats_)) {
      formats_.clear();
      throw std::exception("Sampler with other resources/Root Constant with "
                           "others/More than one unbound binding");
    }
    if (formats.empty()) {
      throw std::exception("Need at least one BindFormat in vector");
    }
    bind_slot_name_ = bind_slot_name;
    if (formats.size()) {
      visibility_ = formats[0].visiblity_;
      for (auto &format : formats_) {
        if (format.visiblity_ != visibility_) {
          visibility_ = ShaderType::SHADER_TYPE_ALL;
        }
        if (format.resource_count_ == 0)
          isUnbound = true;
        bind_count_ += format.resource_count_;
      }
    }
  }

  BindSlot(const std::vector<BindFormat> &formats) : formats_(formats) {
    if (!checkAndMoveUnbound(formats_)) {
      formats_.clear();
      throw std::exception("Sampler with other resources/Root Constant with "
                           "others/More than one unbound binding");
    }
    if (formats.empty()) {
      throw std::exception("Need at least one Bind format in vector");
    }
    bind_slot_name_ = formats[0].name_;
    if (formats.size()) {
      visibility_ = formats[0].visiblity_;
      for (auto &format : formats_) {
        if (format.visiblity_ != visibility_)
          visibility_ = ShaderType::SHADER_TYPE_ALL;
        if (format.resource_count_ == 0)
          isUnbound = true;
        bind_count_ += format.resource_count_;
      }
    }
  }
  // std::shared_ptr<Resource::Resource> bind_resrouce_;
};
// only have query and bind for user, setting is done once generate
// can consider update in future, but this might also effect created pipeline

// not a thread safe object for now!!! need to consider more
class BindLayout {
private:
  std::vector<BindSlot> bind_layout_;
  std::vector<std::shared_ptr<Resource::Resource>> prebind_resources_;
  std::unordered_map<std::string, unsigned int> name_table_;
  std::vector<bool> direct_bind_;
  ComPtr<BindSignature> bind_signature_;
  std::string name_;
  friend class Renderer;
  friend class Context::GraphicsContext;
  friend class Context::ComputeContext;

public:
  BindLayout(ComPtr<BindSignature> bind_signature,
             const std::vector<BindSlot> &bind_layout,
             const std::vector<bool> &direct_bind);
  void setBindResource(unsigned int layout_index,
                       std::shared_ptr<Resource::Resource> resource);
  void setBindResource(const std::string &slot_name,
                       std::shared_ptr<Resource::Resource> resource);
  // retrun a unsigned int  max if didn't find the name
  unsigned int getSlotIndex(const std::string &slot_name);
  bool isDirectBind(unsigned int slot_index) {
    if (slot_index >= bind_layout_.size()) {
      throw std::exception("Slot_index is out of bind_layout size");
    }
    return direct_bind_[slot_index];
  }
  BindType getFirstBindType(unsigned int slot_index) {
    if (slot_index >= bind_layout_.size()) {
      throw std::exception("Slot_index is out of bind_layout size");
    }
    return bind_layout_[slot_index].formats_[0].type_;
  }
  void setName(std::string_view name);
  const BindSlot &getBindSlot(uint32_t index) const {
    return bind_layout_[index];
  }
};

} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine