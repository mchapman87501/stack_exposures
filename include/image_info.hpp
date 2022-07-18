#pragma once

#include <filesystem>
#include <memory>

#include "libraw.h"
#include "shared_ptrs.hpp"
#include <opencv2/core.hpp>

namespace StackExposures {
struct ImageInfo {
  using Ptr = std::shared_ptr<ImageInfo>;

  /**
   * @brief      Constructs a new instance.
   *
   * @param[in]  path   Path of the image file
   * @param      image  OpenCV image data for the image
   */
  ImageInfo(const std::filesystem::path &path, cv::Mat &image);

  /**
   * @brief      Constructs a new instance.
   *
   * @param[in]  processor  Libraw processor that was used to load raw_img
   * @param[in]  path       Path from which raw_img was loaded
   * @param      raw_img    Libraw's representation of the image
   */
  ImageInfo(LibRawPtr processor, const std::filesystem::path &path,
            libraw_processed_image_t *raw_img);

  /**
   * @brief      Constructs a copy of src with updated OpenCV image data.
   *
   * @param[in]  src    ImageInfo instance with image metainfo
   * @param      image  New OpenCV image data
   */
  ImageInfo(const ImageInfo &src, cv::Mat &image);

  ImageInfo(const ImageInfo &src) = delete;
  ImageInfo(ImageInfo &&src) = delete;
  ImageInfo &operator=(const ImageInfo &src) = delete;
  ImageInfo &operator=(ImageInfo &&src) = delete;

  /**
   * @brief      Get the pathname associated with this image.
   *
   * @return     The pathname with which this image is associated
   */
  const std::filesystem::path &path() const;

  /**
   * @brief      Find out whether other_info has the same dimensions as this
   * instance.
   *
   * @param[in]  other_info  another ImageInfo instance
   *
   * @return     true iff this and other_info have the same width and height
   */
  bool same_extents(ImageInfo::Ptr other_info) const;

  /**
   * @brief      Get the height (number of rows) of this instance.
   *
   * @return     The number of rows of this instance's image()
   */
  size_t rows() const;

  /**
   * @brief      Get the width (number of columns) of this instance.
   *
   * @return     The number of columns of this instance's image()
   */
  size_t cols() const;

  /**
   * @brief      Get the OpenCV image data for this instance.
   *
   * @return     The OpenCV image data
   */
  const cv::Mat &image() const;

private:
  const std::filesystem::path m_path;
  LibRawProcessedImagePtr m_raw_img;
  cv::Mat m_image;
};
} // namespace StackExposures
