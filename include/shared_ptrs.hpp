#pragma once

#include "libraw.h"
#include <memory>

namespace StackExposures {
using LibRawSharedPtr = std::shared_ptr<LibRaw>;
using LibRawProcessedImageSharedPtr = std::shared_ptr<libraw_processed_image_t>;
} // namespace StackExposures