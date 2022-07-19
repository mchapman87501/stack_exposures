#pragma once

#include "i_image_stacker.hpp"

namespace StackExposures {

namespace ImageStacker {
/**
 * @brief      Get an image stacker that provides the average of its stacked
 * images.
 *
 * @return     A mean-image stacker
 */
IImageStacker::Ptr mean();

/**
 * @brief      Get an image stacker that provides an average of its stacked
 * images, with stretched brightness range.
 *
 * @return     A stretched-brightness stacker
 */
IImageStacker::Ptr stretch();

} // namespace ImageStacker
} // namespace StackExposures