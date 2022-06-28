#pragma once

#include <filesystem>
#include <memory>

#include "libraw.h"
#include <opencv2/core.hpp>

namespace StackExposures {
struct ImageInfo {
  using UniquePtr = std::unique_ptr<ImageInfo>;
  std::filesystem::path m_path;
  cv::Mat m_image;

  ImageInfo(const std::filesystem::path &path) : m_path(path) {}

  ~ImageInfo();

  LibRaw &processor() { return m_processor; }

  void set_raw_image(libraw_processed_image_t *new_value);

private:
  LibRaw m_processor;
  libraw_processed_image_t *m_raw_img;
};
} // namespace StackExposures
