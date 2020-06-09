#include "../ClassName.h"

#include "RenderTargetSetup.h"
#include <string>
namespace CHCEngine {
namespace Renderer {
namespace Pipeline {

void checkSize(size_t size) {
  if (size > render_targets_limits_) {
    throw std::exception(
        (std::string("Too many render targets, current size is ") +
         std::to_string(size) +
         "  maximum size is :" + std::to_string(render_targets_limits_))
            .c_str());
  }
}
RenderTargetSetup::RenderTargetSetup(const std::vector<DataFormat> &formats) {
  checkSize(formats.size());
  for (int i = 0 ; i < formats.size() ; ++i) {
    render_formats_[i] = formats[i];
  }
}
RenderTargetSetup::RenderTargetSetup(
    const std::vector<std::pair<DataFormat, BlendState>> &formats) {
  checkSize(formats.size());
  for (int i = 0; i < formats.size(); ++i) {
    render_formats_[i] = formats[i].first;
    blend_states_[i] = formats[i].second;
  }
}
RenderTargetSetup::RenderTargetSetup(
    unsigned int count, const std::pair<DataFormat, BlendState> &formats) {
  checkSize(count);
  for (unsigned int i = 0; i < count; ++i) {
    render_formats_[i] = formats.first;
    blend_states_[i] = formats.second;
  }
}
void RenderTargetSetup::setFormat(unsigned int index,
                                  const DataFormat &formats) {
  checkSize(index);
  render_formats_[index] = formats;
}
void RenderTargetSetup::setBlendState(unsigned int index,
                                      const BlendState &state) {
  checkSize(index);
  blend_states_[index] = state;
}
RenderTargetSetup::RenderTargetSetup(unsigned int count,
                                     const DataFormat &formats) {
  checkSize(count);
  for (unsigned int i = 0; i < count; ++i) {
    render_formats_[i] = formats;
  }

}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine