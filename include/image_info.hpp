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

  ImageInfo(const std::filesystem::path &path, cv::Mat &image);
  ImageInfo(LibRawPtr processor, const std::filesystem::path &path,
            libraw_processed_image_t *raw_img);

  /**
   * @brief Construct from src, but with the given OpenCV image.
   */
  ImageInfo(const ImageInfo &src, cv::Mat &image);

  const std::filesystem::path &path() const;

  bool same_extents(ImageInfo::Ptr other_info) const;
  size_t rows() const;
  size_t cols() const;

  const cv::Mat &image() const;

private:
  const std::filesystem::path m_path;
  LibRawProcessedImagePtr m_raw_img;
  cv::Mat m_image;
};
} // namespace StackExposures
