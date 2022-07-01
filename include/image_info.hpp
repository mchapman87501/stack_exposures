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

  ~ImageInfo();

  const std::filesystem::path& path() const { return m_path; }
  cv::Mat &image() { return m_image; }

  bool same_extents(ImageInfo::Ptr other_info) const {
    const cv::Mat &other(other_info->image());
    return ((m_image.rows == other.rows) && (m_image.cols == other.cols));
  }
  size_t rows() const { return m_image.rows; }
  size_t cols() const { return m_image.cols; }

  void update_image(cv::Mat &new_value) { m_image = new_value; }

private:
  const std::filesystem::path m_path;
  LibRawPtr m_processor;
  libraw_processed_image_t *m_raw_img;
  cv::Mat m_image;
};
} // namespace StackExposures
