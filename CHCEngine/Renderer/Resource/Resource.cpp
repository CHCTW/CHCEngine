#include "../ClassName.h"

#include "Resource.h"
//#include "../Pipeline/BindLayout.h"

namespace CHCEngine {
namespace Renderer {
namespace Resource {
void Resource::setName(std::string_view name) {
  std::string temp(name);
  NAME_D3D12_OBJECT_STRING(gpu_resource_, temp);
  information_.name_ = name;
}
} // namespace Resource
} // namespace Renderer
} // namespace CHCEngine