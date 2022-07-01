#pragma once

#include "libraw.h"
#include <memory>

namespace StackExposures {
using LibRawPtr = std::shared_ptr<LibRaw>;
using LibRawProcessedImagePtr = std::shared_ptr<libraw_processed_image_t>;
} // namespace StackExposures