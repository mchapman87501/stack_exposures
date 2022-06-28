#pragma once

#include <filesystem>
#include <opencv2/core.hpp>

namespace StackExposures {
struct ImageInfo {
  std::filesystem::path m_path;
  cv::Mat m_image;
};

class AlignedImageGenerator {
public:
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

  ImageInfo align(const std::filesystem::path &image_path);
};
} // namespace StackExposures
