#pragma once

#include <memory>

#import "image_info.hpp"

#include "libraw.h"

namespace StackExposures {
class ImageLoader {
  using Ptr = std::shared_ptr<ImageLoader>;

public:
  ImageLoader();

  ImageInfo::Ptr load_image(const std::filesystem::path &image_path);

private:
  LibRawPtr m_processor;

  void check(int status, const std::string &msg);

  ImageInfo::Ptr load_raw_image(const std::filesystem::path &image_path);
};
} // namespace StackExposures