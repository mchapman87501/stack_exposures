#pragma once

#include "image_info.hpp"

namespace StackExposures {
struct ImageAligner {
  /**
   * @brief Align an image to all other images aligned so far.
   *
   * @param image The image to align.
   *              If this is the first image aligned, it becomes the reference
   *              image against which all others are aligned.
   * @return The aligned image, or in case of processing
   * problems, the original image
   */
  ImageInfo::SharedPtr align(ImageInfo::SharedPtr image);

private:
  ImageInfo::SharedPtr m_ref_img = nullptr;

  ImageInfo::SharedPtr align_internal(ImageInfo::SharedPtr ref,
                                      ImageInfo::SharedPtr to_align);
};
} // namespace StackExposures
