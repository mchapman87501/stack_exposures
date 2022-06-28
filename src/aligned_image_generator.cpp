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

} // namespace

namespace StackExposures {
AlignedImageGenerator::AlignedImageGenerator()
    : m_processor(std::make_shared<LibRaw>()) {}

ImageInfo::Ptr
AlignedImageGenerator::align(const std::filesystem::path &image_path) {
  ImageInfo::Ptr result = load_image(image_path);

  if (!m_ref_img) {
    m_ref_img = result;
  } else {
    result = align_internal(m_ref_img, result);
  }
  return result;
}

ImageInfo::Ptr
AlignedImageGenerator::load_image(const std::filesystem::path &image_path) {
  ImageInfo::Ptr result =
      std::make_shared<ImageInfo>(ImageInfo(m_processor, image_path));

  check(m_processor->open_file(image_path.c_str()), "Could not open file");
  check(m_processor->unpack(), "Could not unpack");
  check(m_processor->raw2image(), "raw2image");
  check(m_processor->dcraw_process(),
        "dcraw_process"); // This is what Rawpy uses.

  // To extract image data, rawpy uses
  // dcraw_make_mem_image().
  int status;
  libraw_processed_image_t *img = m_processor->dcraw_make_mem_image(&status);
  check(status, "dcraw_make_mem_image");
  assert(img);
  assert(img->type == LIBRAW_IMAGE_BITMAP);

  result->set_raw_image(img);

  return result;
}

ImageInfo::Ptr AlignedImageGenerator::align_internal(ImageInfo::Ptr ref,
                                                     ImageInfo::Ptr unaligned) {
  return unaligned;
}
} // namespace StackExposures
