#pragma once

#include "image_info.hpp"

namespace StackExposures {
class ImageStacker {
public:
  ImageStacker();

  /**
   * @brief Push a new image onto the stack.
   * 
   * @details If the dimensions of the new image do not match the rest of the 
   * stack, the image will not be stacked.
   *
   * @param[in]  new_image  The image to be stacked up.
   */
  void push(const cv::Mat &new_image);

  /**
   * @brief Get the stacked composite image.
   *
   * @return an OpenCV image
   */
  cv::Mat result();

private:
  size_t m_width;
  size_t m_height;
  cv::Mat m_image;
};
} // namespace StackExposures