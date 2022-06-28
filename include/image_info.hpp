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
  cv::Mat m_image;

  ImageInfo(LibRawPtr processor, const std::filesystem::path &path)
      : m_processor(processor), m_path(path) {}

  ~ImageInfo();

  void set_raw_image(libraw_processed_image_t *new_value);

private:
  LibRawPtr m_processor;
  libraw_processed_image_t *m_raw_img;
};
} // namespace StackExposures
