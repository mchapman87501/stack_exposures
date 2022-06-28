#pragma once

#include "image_info.hpp"

namespace StackExposures {
class ImageStacker {
public:
  ImageStacker();

  void push(const cv::Mat &new_image);

  /**
   * @brief Get the stacked result as a BGR image.
   *
   * @return cv::Mat
   */
  cv::Mat result();

private:
  size_t m_width;
  size_t m_height;
  cv::Mat m_image;
};
} // namespace StackExposures