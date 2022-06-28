#include "aligned_image_generator.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

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
    align_internal(m_ref_img, result);
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

void AlignedImageGenerator::align_internal(ImageInfo::Ptr ref,
                                           ImageInfo::Ptr to_align) {
  // See
  // https://docs.opencv.org/4.6.0/dd/d93/samples_2cpp_2image_alignment_8cpp-example.html#a39
  using namespace cv;
  Mat ref_gray;
  cvtColor(ref->image(), ref_gray, COLOR_BGR2GRAY);

  Mat to_align_gray;
  cvtColor(to_align->image(), to_align_gray, COLOR_BGR2GRAY);

  const auto warp_mode = MOTION_TRANSLATION;
  const int num_iterations = 20000;
  const double termination_eps = 1.0e-9;
  Mat warp_matrix = Mat::eye(2, 3, CV_32F);
  double correlation_val =
      findTransformECC(ref_gray, to_align_gray, warp_matrix, warp_mode,
                       TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
                                    num_iterations, termination_eps));

  // Do the alignment.
  Mat warped_image =
      Mat(to_align->image().rows, to_align->image().cols, CV_32FC1);
  warpAffine(to_align->image(), warped_image, warp_matrix, warped_image.size(),
             INTER_LINEAR + WARP_INVERSE_MAP);
  to_align->update_image(warped_image);
}
} // namespace StackExposures
