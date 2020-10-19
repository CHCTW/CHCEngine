#include "../ClassName.h"

#include "../Resource/Buffer.h"
#include "../Resource/DynamicBuffer.h"
#include "../Resource/Texture.h"
#include "Context.h"
#include "ContextPool.h"
#include "CopyContext.h"

namespace CHCEngine {
namespace Renderer {

namespace Context {
void CopyContext::updateBuffer(const std::shared_ptr<Resource::Buffer> &buffer,
                               void const *data,
                               unsigned long long data_byte_size,
                               unsigned long long offset) {
  if (!buffer) {
    throw std::exception("Can't update buffer with null buffer pointer");
  }
  if (!data) {
    throw std::exception("Can't update buffer with null pointer");
  }
  if (data_byte_size + offset > buffer->getBufferInformation().size_) {
    throw std::exception("Update data + offset is out of buffer range");
  }
  // copy queue will auto decay to common, so not transiton will be used this
  // context
  updateContextResourceState(buffer, ResourceState::RESOURCE_STATE_COPY_DEST,
                             false, 0);
  flushBarriers();
  if (buffer->getInformation().update_type_ ==
      Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC) {
    context_command_->updateBufferRegion(buffer, data, data_byte_size, offset);
  } else {
    auto space = context_command_->requestSpace(data_byte_size);
    context_command_->updateBufferRegion(buffer, data, data_byte_size, offset,
                                         space);
    /*if (auto pool = pool_.lock()) {
      auto space = pool->getDynamicUploadBuffer()->reqeustSpace(data_byte_size);
      // auto space = context_command_->requestSpace(data_byte_size);
      context_command_->updateBufferRegion(buffer, data, data_byte_size, offset,
                                           space);
    } else {
      throw std::exception(
          "Context Pool already deleted, can't get dynamic upload buffer");
    }*/
  }
}
inline const char *
fillCopyLayoutAndCopy(D3D12_TEXTURE_COPY_LOCATION &src_copy_layout,
                      const ComPtr<GPUResource> &src_gpu,
                      unsigned long long src_offset, char *src_cpu_point,
                      const char *data, unsigned int row_count,
                      unsigned long long row_byte_size,
                      D3D12_PLACED_SUBRESOURCE_FOOTPRINT &texture_footprint) {
  src_copy_layout.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  src_copy_layout.pResource = src_gpu.Get();
  src_copy_layout.PlacedFootprint.Offset = src_offset;
  src_copy_layout.PlacedFootprint.Footprint = texture_footprint.Footprint;
  unsigned long long gpu_row_byte_size = texture_footprint.Footprint.RowPitch;
  for (unsigned int i = 0; i < row_count; ++i) {
    memcpy(src_cpu_point, data, row_byte_size);
    data += row_byte_size;
    src_cpu_point += gpu_row_byte_size;
  }
  return data;
}
void CopyContext::updateTexture(
    const std::shared_ptr<Resource::Texture> &texture,
    const void *consecutive_image_data, unsigned int array_start_index,
    unsigned int array_count, unsigned int mip_start_level,
    unsigned int mip_count) {
  auto inf = texture->getTextureInformation();
  auto update_type = texture->getInformation().update_type_;
  unsigned int array_size = inf.depth_;
  if (inf.type_ == TextureType::TEXTURE_TYPE_3D) {
    array_size = 1;
    array_start_index = 0;
  }
  if (array_start_index >= array_size) {
    throw std::exception(
        (std::string("Can't update texture with array start index : ") +
         std::to_string(array_start_index) +
         " the texture depth is : " + std::to_string(array_size))
            .c_str());
  }
  if (mip_start_level >= inf.depth_) {
    throw std::exception(
        (std::string("Can't update texture with mip start level : ") +
         std::to_string(mip_start_level) +
         " the texture depth is : " + std::to_string(inf.mip_levels_))
            .c_str());
  }

  array_count = (std::min)(array_count, array_size - array_start_index);
  mip_count = (std::min)(mip_count, inf.mip_levels_ - mip_start_level);

  uint32_t total = array_count * mip_count;
  if (total == texture->getSubResrouceCount()) {
    updateContextResourceState(texture, ResourceState::RESOURCE_STATE_COPY_DEST,
                               false, all_subresrouce_index);
  } else {
    // loop update every sub resource state
    for (unsigned int i = array_start_index;
         i < array_start_index + array_count; ++i) {
      for (unsigned int j = mip_start_level; j < mip_start_level + mip_count;
           ++j) {
        unsigned int sub_index = i * inf.mip_levels_ + j;
        updateContextResourceState(
            texture, ResourceState::RESOURCE_STATE_COPY_DEST, false, sub_index);
      }
    }
  }
  flushBarriers();

  // need to consider 3d texture
  std::vector<D3D12_TEXTURE_COPY_LOCATION> src_copy_layouts_(
      static_cast<unsigned long long>(array_count) *
      static_cast<unsigned long long>(mip_count));
  std::vector<D3D12_TEXTURE_COPY_LOCATION> dest_copy_layouts_(
      static_cast<unsigned long long>(array_count) *
      static_cast<unsigned long long>(mip_count));
  std::vector<std::shared_ptr<Resource::AllocateSpace>> allocate_spaces(
      static_cast<unsigned long long>(array_count) *
      static_cast<unsigned long long>(mip_count));
  unsigned int copy_index = 0;
  unsigned long long data_offset = 0;
  const char *char_data = static_cast<const char *>(consecutive_image_data);
  auto pool = pool_.lock();
  for (unsigned int i = array_start_index; i < array_start_index + array_count;
       ++i) {
    for (unsigned int j = mip_start_level; j < mip_start_level + mip_count;
         ++j) {
      unsigned int sub_index = i * inf.mip_levels_ + j;
      unsigned int row_count = inf.foot_prints_[sub_index].Footprint.Height *
                               inf.foot_prints_[sub_index].Footprint.Depth;
      dest_copy_layouts_[copy_index].Type =
          D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
      dest_copy_layouts_[copy_index].SubresourceIndex = sub_index;
      dest_copy_layouts_[copy_index].pResource = texture->gpu_resource_.Get();
      // dynamic
      if (update_type ==
          Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC) {
        char_data = fillCopyLayoutAndCopy(
            src_copy_layouts_[copy_index], texture->upload_buffer_,
            inf.foot_prints_[sub_index].Offset,
            static_cast<char *>(texture->upload_buffer_map_pointer_) +
                inf.foot_prints_[sub_index].Offset,
            char_data, row_count, inf.row_byte_sizes_[sub_index],
            inf.foot_prints_[sub_index]);
      } else {
        // if (pool) {
        // allocate subresource size memory
        unsigned long long require_space =
            static_cast<unsigned long long>(row_count) *
            static_cast<unsigned long long>(
                inf.foot_prints_[sub_index].Footprint.RowPitch);
        // add more 512 byte since we need offset can be 512 byte algiment
        // allocate_spaces[copy_index] =
        //   pool->getDynamicUploadBuffer()->reqeustSpace(
        //      require_space + texture_subresouce_offset_aligment);
        allocate_spaces[copy_index] = context_command_->requestSpace(
            require_space + texture_subresouce_offset_aligment);
        char *copy_pont =
            static_cast<char *>(allocate_spaces[copy_index]->copy_point_);
        unsigned long long alignment_gpu_offset =
            allocate_spaces[copy_index]->gpu_offset_;
        alignment_gpu_offset =
            (alignment_gpu_offset + (texture_subresouce_offset_aligment - 1)) &
            ~(texture_subresouce_offset_aligment - 1);
        copy_pont +=
            (alignment_gpu_offset - allocate_spaces[copy_index]->gpu_offset_);
        char_data = fillCopyLayoutAndCopy(
            src_copy_layouts_[copy_index], allocate_spaces[copy_index]->buffer_,
            alignment_gpu_offset, copy_pont, char_data, row_count,
            inf.row_byte_sizes_[sub_index], inf.foot_prints_[sub_index]);
        /*} else {
          throw std::exception("Context Pool already deleted, can't get "
                               "dynamic upload buffer");
        }*/
      }
      ++copy_index;
    }
  }
  if (update_type ==
      Resource::ResourceUpdateType::RESOURCE_UPDATE_TYPE_DYNAMIC) {
    context_command_->updateTextureRegion(texture, src_copy_layouts_,
                                          dest_copy_layouts_);
  } else {
    context_command_->updateTextureRegion(texture, src_copy_layouts_,
                                          dest_copy_layouts_, allocate_spaces);
  }
}
} // namespace Context
} // namespace Renderer
} // namespace CHCEngine