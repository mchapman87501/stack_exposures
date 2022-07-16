#pragma once

#include "image_info.hpp"
#include <memory>

namespace StackExposures {
struct IImageStacker {
  using Ptr = std::shared_ptr<IImageStacker>;

  /**
   * @brief Push a new image onto the stack.
   *
   * @details If the dimensions of the new image do not match the rest of the
   * stack, the image will not be stacked.
   *
   * @param[in]  new_image  The image to be stacked up.
   */
  virtual void push(const cv::Mat &new_image) = 0;

  /**
   * @brief Get the stacked composite image.
   *
   * @return cv::Mat An 8-bit, 3 channel image
   */
  virtual cv::Mat result8() const = 0;

  /**
   * @brief Get the stacked composite image.
   *
   * @return cv::Mat A 16-bit, 3-channel image
   */
  virtual cv::Mat result16() const = 0;

  /**
   * @brief Get the stacked composite image.
   *
   * @return cv::Mat A 64-bit, floating point, 3-channel image
   */
  virtual cv::Mat resultf() const = 0;
};
} // namespace StackExposures