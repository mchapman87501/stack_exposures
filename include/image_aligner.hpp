#pragma once

#include <filesystem>
#include <memory>

#include "image_info.hpp"
#include "shared_ptrs.hpp"

namespace StackExposures {
class ImageAligner {
public:
  using Ptr = std::shared_ptr<ImageAligner>;

  ImageAligner();

  /**
   * @brief Align an image to all other images aligned so far.
   *
   * @param image The image to align.
   *              If this is the first image aligned, it becomes the reference
   *              image against which all others are aligned.
   * @return ImageInfo::Ptr the aligned image
   */
  ImageInfo::Ptr align(ImageInfo::Ptr image);

private:
  ImageInfo::Ptr m_ref_img = nullptr;

  ImageInfo::Ptr align_internal(ImageInfo::Ptr ref, ImageInfo::Ptr to_align);
};
} // namespace StackExposures
