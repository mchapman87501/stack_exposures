#pragma once

#include <filesystem>
#include <memory>

#include "libraw.h"
#include "shared_ptrs.hpp"
#include <opencv2/core.hpp>

namespace StackExposures {
struct ImageInfo {
  using SharedPtr = std::shared_ptr<ImageInfo>;

  /**
   * @brief      Create with an image loaded from a file.
   *
   * @param[in]  path   Path to the image file
   * @param      image  The image loaded from the file
   *
   * @return     A shared pointer to the new instance
   */
  static SharedPtr from_file(std::filesystem::path path, cv::Mat &image);

  /**
   * @brief      Create with an image loaded from a raw file.
   *
   * @param[in]  processor  The LibRaw instance used to process the image
   * @param[in]  path       Path to the image file
   * @param      raw_img    LibRaw processed image
   *
   * @return     A shared pointer to the new instance
   */
  static SharedPtr from_raw_file(LibRawSharedPtr processor,
                                 std::filesystem::path path,
                                 libraw_processed_image_t *raw_img);

  /**
   * @brief      Create a variation of an existing instance.
   *
   * @param[in]  src    The instance, a variation of which to create
   * @param      image  The image to associate with the variation
   *
   * @return     A shared pointer to the new instance
   */
  static SharedPtr with_image(const ImageInfo &src, cv::Mat &image);

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
  bool same_extents(ImageInfo::SharedPtr other_info) const;

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

protected:
  /**
   * @brief      Constructs a new instance.
   *
   * @param[in]  path   Path of the image file
   * @param      image  OpenCV image data for the image
   */
  ImageInfo(std::filesystem::path path, cv::Mat &image);

  /**
   * @brief      Constructs a new instance.
   *
   * @param[in]  processor  Libraw processor that was used to load raw_img
   * @param[in]  path       Path from which raw_img was loaded
   * @param      raw_img    Libraw's representation of the image
   */
  ImageInfo(LibRawSharedPtr processor, std::filesystem::path path,
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

private:
  const std::filesystem::path m_path;
  LibRawProcessedImageSharedPtr m_raw_img;
  cv::Mat m_image;
};
} // namespace StackExposures
