#pragma once

#include "i_image_stacker.hpp"

namespace StackExposures {
struct ImageStacker : public IImageStacker {
  using Ptr = std::shared_ptr<ImageStacker>;

  static Ptr create();
};
} // namespace StackExposures