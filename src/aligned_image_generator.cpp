#include <sstream>
#include <stdexcept>
#include <string>

#include "aligned_image_generator.hpp"

namespace {
using namespace StackExposures;

void check(int status, const std::string &msg) {
  using namespace std;

  if (LIBRAW_FATAL_ERROR(status)) {
    ostringstream outs;
    outs << "Fatal error: " << msg;
    throw new runtime_error(outs.str());
  }
  if (LIBRAW_SUCCESS != status) {
    throw new runtime_error(msg);
  }
}

ImageInfo::UniquePtr load_image(const std::filesystem::path &image_path) {
  ImageInfo::UniquePtr result =
      std::make_unique<ImageInfo>(ImageInfo(image_path));

  LibRaw &processor(result->processor());

  check(processor.open_file(image_path.c_str()), "Could not open file");
  check(processor.unpack(), "Could not unpack");
  check(processor.raw2image(), "raw2image");
  check(processor.dcraw_process(), "dcraw_process"); // This is what Rawpy uses.

  // To extract image data, rawpy uses
  // dcraw_make_mem_image().
  int status;
  libraw_processed_image_t *img = processor.dcraw_make_mem_image(&status);
  check(status, "dcraw_make_mem_image");
  assert(img);
  assert(img->type == LIBRAW_IMAGE_BITMAP);

  result->set_raw_image(img);

  return result;
}
} // namespace

namespace StackExposures {
ImageInfo::UniquePtr
AlignedImageGenerator::align(const std::filesystem::path &image_path) {
  // XXX FIX THIS I should own the LibRaw instance.
  ImageInfo::UniquePtr result = load_image(image_path);
  // TODO Align the new image to the reference image if any;
  // otherwise record the result as the reference image.
  return result;
}
} // namespace StackExposures
