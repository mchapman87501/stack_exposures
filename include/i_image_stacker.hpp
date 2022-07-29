#pragma once

#include "image_info.hpp"
#include <memory>

namespace StackExposures {
struct IImageStacker {
  using Ptr = std::unique_ptr<IImageStacker>;

  virtual ~IImageStacker() = default;

  /**
   * @brief Add a new image to the exposure.
   *
   * @details If the dimensions of the new image do not match the rest of the
   * stack, the image will not be added.
   *
   * @param  new_image  The image to be added.
   */
  virtual void add(const cv::Mat &new_image) = 0;

  /**
   * @brief  Subtract an image -- e.g., a dark frame -- from the stack.
   *
   * @param  new_image  The image to be subtracted.
   */
  virtual void subtract(const cv::Mat &new_image) = 0;

  /**
   * @brief      Get the result of stacking so far, without dark image
   * subtraction, averaging, etc.
   *
   * @return     The sum of all images stacked so far, in 32FC3 format.
   */
  [[nodiscard]] virtual cv::Mat partial_sum() const = 0;

  /**
   * @brief Get the stacked composite image.
   *
   * @return cv::Mat An 8-bit, 3 channel image
   */
  [[nodiscard]] virtual cv::Mat result8() const = 0;

  /**
   * @brief Get the stacked composite image.
   *
   * @return cv::Mat A 16-bit, 3-channel image
   */
  [[nodiscard]] virtual cv::Mat result16() const = 0;
};
} // namespace StackExposures