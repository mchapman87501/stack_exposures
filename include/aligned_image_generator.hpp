#pragma once

#include <filesystem>
#include <memory>

#include "image_info.hpp"

namespace StackExposures {
class AlignedImageGenerator {
public:
  using UniquePtr = std::unique_ptr<AlignedImageGenerator>;

  /**
   * @brief Align an(other) image.
   *
   * Aligns the image at 'image_path' with all images aligned so far.
   * If no images have been aligned, this becomes the reference image
   * against which all subsequent calls to 'align' will align.
   *
   * @param[in] image_path the path to the image (including raw) to align
   *
   * @return ImageInfo for the aligned image.
   */

  ImageInfo::UniquePtr align(const std::filesystem::path &image_path);

protected:
  ImageInfo::UniquePtr load_image(const std::filesystem::path &image_path);
};
} // namespace StackExposures
