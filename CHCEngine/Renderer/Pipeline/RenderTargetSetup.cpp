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
  for (int i = 0; i < formats.size(); ++i) {
    render_formats_[i] = formats[i];
  }
  size_ = static_cast<unsigned int>(formats.size());
}
RenderTargetSetup::RenderTargetSetup(
    const std::vector<std::pair<DataFormat, BlendState>> &formats) {
  checkSize(formats.size());
  for (int i = 0; i < formats.size(); ++i) {
    render_formats_[i] = formats[i].first;
    blend_states_[i] = formats[i].second;
  }
  size_ = static_cast<unsigned int>(formats.size());
}
RenderTargetSetup::RenderTargetSetup(
    unsigned int count, const std::pair<DataFormat, BlendState> &formats) {
  checkSize(count);
  for (unsigned int i = 0; i < count; ++i) {
    render_formats_[i] = formats.first;
    blend_states_[i] = formats.second;
  }
  size_ = static_cast<unsigned int>(count);
}
void RenderTargetSetup::setFormat(unsigned int index,
                                  const DataFormat &formats) {
  if (index>=size_) {
    throw std::exception(
        (std::string("Incorrect Blend Index :") + std::to_string(index) +
                          "  current  size is :" + std::to_string(size_))
            .c_str());
  }
  checkSize(static_cast<size_t>(index) + 1u);
  render_formats_[index] = formats;
}
void RenderTargetSetup::setBlendState(unsigned int index,
                                      const BlendState &state) {
  checkSize(static_cast<size_t>(index) + 1u);
  if (index >= size_) {
    throw std::exception((std::string("Incorrect Blend Index :") +
                          std::to_string(index) +
                          "  current  size is :" + std::to_string(size_))
                             .c_str());
  }
  blend_states_[index] = state;
}
RenderTargetSetup::RenderTargetSetup(unsigned int count,
                                     const DataFormat &formats) {
  checkSize(count);
  for (unsigned int i = 0; i < count; ++i) {
    render_formats_[i] = formats;
  }
  size_ = count;
}
} // namespace Pipeline
} // namespace Renderer
} // namespace CHCEngine