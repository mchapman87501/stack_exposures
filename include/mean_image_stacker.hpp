#pragma once

#include "i_image_stacker.hpp"

namespace StackExposures {
struct MeanImageStacker : public IImageStacker {
  using Ptr = std::unique_ptr<MeanImageStacker>;

  static Ptr create();
  virtual ~MeanImageStacker() = default;
};
} // namespace StackExposures