#pragma once

#include "i_image_stacker.hpp"

namespace StackExposures {

namespace ImageStacker {
IImageStacker::Ptr mean();
IImageStacker::Ptr stretch();
} // namespace ImageStacker
} // namespace StackExposures