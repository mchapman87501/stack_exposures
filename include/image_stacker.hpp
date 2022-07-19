#pragma once

#include "i_image_stacker.hpp"

namespace StackExposures {
struct ImageStacker : public IImageStacker {
  using Ptr = std::unique_ptr<ImageStacker>;

  static Ptr create();
  virtual ~ImageStacker() = default;
};
} // namespace StackExposures