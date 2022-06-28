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

  std::filesystem::path m_path;

  ImageInfo(LibRawPtr processor, const std::filesystem::path &path,
            libraw_processed_image_t *raw_img);

  ~ImageInfo();

  cv::Mat &image() { return m_image; }
  void update_image(cv::Mat &new_value) { m_image = new_value; }

private:
  LibRawPtr m_processor;
  libraw_processed_image_t *m_raw_img;
  cv::Mat m_image;
};
} // namespace StackExposures
