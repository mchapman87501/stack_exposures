#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "image_aligner.hpp"
#include "image_info.hpp"

namespace StackExposures {

using ImageInfoFutureContainer = std::vector<ImageInfoFuture>;

struct ImageStacker {
  using Ptr = std::unique_ptr<ImageStacker>;

  virtual ~ImageStacker() = default;

  static Ptr create();

  [[nodiscard]] virtual cv::Mat
  stacked_result(const ImageInfoFutureContainer &images,
                 ImageInfo::SharedPtr dark_image = nullptr,
                 bool align = true) const = 0;
};

} // namespace StackExposures
