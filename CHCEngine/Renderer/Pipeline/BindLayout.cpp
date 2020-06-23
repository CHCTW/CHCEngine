#include "../ClassName.h"

#include "BindLayout.h"
#include <algorithm>
#include <limits>
namespace CHCEngine {
namespace Renderer {
namespace Pipeline {
bool BindSlot::checkAndMoveUnbound(std::vector<BindFormat> &formats) {
  bool findsample = false;
  bool findother = false;
  bool findunbound = false;
  unsigned int unbound_index = 0;
  unsigned int current = 0;
  for (auto &format : formats) {
    // root constants can only be one slot
    if (format.type_ == BindType::BIND_TYPE_SIT_ROOT_CONSTANT &&
        formats.size() != 1) {
      return false;
    }
    if (format.type_ == BindType::BIND_TYPE_SIT_SAMPLER) {
      if (findother == true) {
        return false;
      }
      findsample = true;
    } else {
      if (findsample == true) {
        return false;
      }
      findother = true;
    }
    if (format.resource_count_ == 0) {
      if (findunbound) {
        return false;
      }
      findunbound = true;
      unbound_index = current;
    }
    ++current;
  }
  // keep unbound at the end
  if (findunbound) {
    std::swap(formats[unbound_index], formats[formats.size() - 1]);
  }
  return true;
}
BindLayout::BindLayout(ComPtr<BindSignature> bind_signature,
                       const std::vector<BindSlot> &bind_layout,
                       const std::vector<bool> &direct_bind) {
  for (int i = 0; i < bind_layout.size(); ++i) {
    if (name_table_.count(bind_layout[i].bind_slot_name_)) {
      name_table_.clear();
      throw std::exception("Have the same slot name bind layer");
    }
    name_table_[bind_layout[i].bind_slot_name_] = i;
  }
  prebind_resources_.resize(bind_layout.size());
  bind_signature_ = std::move(bind_signature);
  bind_layout_ = bind_layout;
  direct_bind_ = direct_bind;
}
void BindLayout::setBindResource(unsigned int layout_index,
                                 std::shared_ptr<Resource::Resource> resource) {
  if (layout_index >= bind_layout_.size()) {
    throw std::exception("Index is out of bind_layout_ size ");
  }
  prebind_resources_[layout_index] = std::move(resource);
}
void BindLayout::setBindResource(const std::string &slot_name,
                                 std::shared_ptr<Resource::Resource> resource) {
  if (name_table_.count(slot_name)) {
    prebind_resources_[name_table_[slot_name]] = std::move(resource);
  } else {
    throw std::exception("Can't find slot name in bind layout");
  }
}
unsigned int BindLayout::getSlotIndex(const std::string &slot_name) {
  if (name_table_.count(slot_name))
    return name_table_[slot_name];
  return (std::numeric_limits<unsigned int>::max)();
}
void BindLayout::setName(std::string_view name) {
  std::string temp(name);
  NAME_D3D12_OBJECT_STRING(bind_signature_, temp);
  name_ = name;
}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine