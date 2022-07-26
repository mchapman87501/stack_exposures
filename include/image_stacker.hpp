#pragma once

#include "i_image_stacker.hpp"

namespace StackExposures {

namespace ImageStacker {
/**
 * @brief      Get an image stacker that provides the average of its stacked
 * images.
 *
 * @param[in] gamma  gamma correction factor, e.g., 2.4.  A value of 1
 * results in no gamma correction.
 *
 * @return     A mean-image stacker
 */
IImageStacker::Ptr mean(double gamma);

/**
 * @brief      Get an image stacker that provides an average of its stacked
 * images, with stretched brightness range.
 *
 * @param[in] gamma  gamma correction factor, e.g., 2.4.  A value of 1 results
 * in no gamma correction.
 *
 * @return     A stretched-brightness stacker
 */
IImageStacker::Ptr stretch(double gamma);

} // namespace ImageStacker
} // namespace StackExposures