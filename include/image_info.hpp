#pragma once

#include <filesystem>
#include <memory>

#include "libraw.h"
#include "shared_ptrs.hpp"
#include <opencv2/core.hpp>

namespace StackExposures {
struct ImageInfo {
  using UniquePtr = std::unique_ptr<ImageInfo>;
  using Ptr = std::shared_ptr<ImageInfo>;

  ImageInfo(const std::filesystem::path &path, cv::Mat &image)
      : m_path(path), m_processor(nullptr), m_raw_img(nullptr), m_image(image) {
  }

  ImageInfo(LibRawPtr processor, const std::filesystem::path &path,
            libraw_processed_image_t *raw_img);

  /**
   * @brief Construct from src, but with the given OpenCV image.
   */
  ImageInfo(const ImageInfo &src, cv::Mat &image);

  ~ImageInfo();

  const std::filesystem::path &path() const;

  bool same_extents(ImageInfo::Ptr other_info) const;
  size_t rows() const;
  size_t cols() const;

  cv::Mat &image();

private:
  const std::filesystem::path m_path;
  LibRawPtr m_processor;
  LibRawProcessedImagePtr m_raw_img;
  cv::Mat m_image;
};
} // namespace StackExposures
