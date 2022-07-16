#pragma once

#include "i_image_stacker.hpp"

namespace StackExposures {
struct MeanImageStacker : public IImageStacker {
  using Ptr = std::shared_ptr<MeanImageStacker>;

  static Ptr create();
};
} // namespace StackExposures