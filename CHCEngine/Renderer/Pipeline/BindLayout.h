#pragma once

#include <memory>
#include <vector>
#include <mutex>

#include "../Resource/Resource.h"
#include "Shader.h"

using Microsoft::WRL::ComPtr;

namespace CHCEngine {
namespace Renderer {
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
  // sampler can't stay in the same slot with other resource
  BindSlot() = delete;
  static bool checkAndMoveUnbound(std::vector<BindFormat> &formats);
  BindSlot(const std::string &bind_slot_name,
           const std::vector<BindFormat> &formats)
      : formats_(formats) {
    if (!checkAndMoveUnbound(formats_)) {
      formats_.clear();
      throw std::exception(
          "Samper with other resrouces/Root Constant with others/More than one unbound binding");
    }
    if (formats.empty()) {
      throw std::exception("Need at least one BindFormat in vector");
    }
    bind_slot_name_ = bind_slot_name;
  }

  BindSlot(const std::vector<BindFormat> &formats) : formats_(formats) {
    if (!checkAndMoveUnbound(formats_)) {
      formats_.clear();
      throw std::exception(
          "Samper with other resrouces/Root Constant with others/More than one unbound binding");
    }
    if (formats.empty()) {
      throw std::exception("Need at least one Bindformat in vector");
    }
    bind_slot_name_ = formats[0].name_;
  }
  // std::shared_ptr<Resource::Resource> bind_resrouce_;
};
// only have query and bind for user, setting is done once generate
// can consider update in future, but this might also effect created pipeline

// not a thread safe object for now!!! need to consider more 
class BindLayout {
private:
  std::vector<BindSlot>
      bind_layout_;
  std::vector<std::shared_ptr<Resource::Resource>> prebind_resources_;
  std::unordered_map<std::string, unsigned int> name_table_;
  ComPtr<BindSignature> bind_signature_;
public:
  BindLayout(ComPtr<BindSignature> bind_signature,
             const std::vector<BindSlot>
                 bind_layout);
  void setBindResource(unsigned int layout_index,
                       std::shared_ptr<Resource::Resource> resource);
  void setBindResource(const std::string &slot_name,
                    std::shared_ptr<Resource::Resource> resource);
  //retrun a unsigned int  max if didn't find the name
  unsigned int getBindIndex(const std::string &slot_name);
};

} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine