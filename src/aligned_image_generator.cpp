#include "aligned_image_generator.hpp"
#include "libraw.h"

namespace StackExposures {
ImageInfo
AlignedImageGenerator::align(const std::filesystem::path &image_path) {
  ImageInfo result{image_path};

    LibRaw processor;
    processor.open_file(image_path.c_str());
    processor.unpack();
    processor.raw2image();
    // TODO convert processor.imgdata to an OpenCV Mat.
  return result;
}
} // namespace StackExposures
