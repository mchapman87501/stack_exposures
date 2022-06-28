#include "aligned_image_generator.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <opencv2/core/utility.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

namespace {
using namespace StackExposures;

} // namespace

namespace StackExposures {
AlignedImageGenerator::AlignedImageGenerator()
    : m_processor(std::make_shared<LibRaw>()) {}

void AlignedImageGenerator::check(int status, const std::string &msg) {
  using namespace std;

  if (LIBRAW_FATAL_ERROR(status)) {
    ostringstream outs;
    outs << "Fatal error: " << msg << "; status = " << status << " ("
         << m_processor->strerror(status) << ")";
    cerr << outs.str() << endl;
    throw new runtime_error(outs.str());
  }
  if (LIBRAW_SUCCESS != status) {
    const char *status_msg =
        (status < 0) ? m_processor->strerror(status) : std::strerror(status);
    cerr << msg << "; status = " << status << " (" << status_msg << ")" << endl;
    throw new runtime_error(msg);
  }
}

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
  cv::Mat image = cv::imread(image_path.c_str());
  if (image.data != nullptr) {
    return std::make_shared<ImageInfo>(image_path, image);
  }
  return load_raw_image(image_path);
}

ImageInfo::Ptr
AlignedImageGenerator::load_raw_image(const std::filesystem::path &image_path) {
  check(m_processor->open_file(image_path.c_str()), "Could not open file");
  check(m_processor->unpack(), "Could not unpack");

  check(m_processor->dcraw_process(),
        "dcraw_process"); // This is what Rawpy uses.

  // To extract image data, rawpy uses
  // dcraw_make_mem_image().
  int status = 0;
  libraw_processed_image_t *img = m_processor->dcraw_make_mem_image(&status);
  check(status, "dcraw_make_mem_image");
  assert(img);
  assert(img->type == LIBRAW_IMAGE_BITMAP);

  ImageInfo::Ptr result =
      std::make_shared<ImageInfo>(m_processor, image_path, img);
  return result;
}

void AlignedImageGenerator::align_internal(ImageInfo::Ptr ref,
                                           ImageInfo::Ptr to_align) {
  // See
  // https://docs.opencv.org/4.6.0/dd/d93/samples_2cpp_2image_alignment_8cpp-example.html#a39

  // Bail immediately if the two images have different sizes.
  if ((ref->image().rows != to_align->image().rows) ||
      (ref->image().cols != to_align->image().cols)) {
    std::cerr << "Cannot align " << to_align->m_path << " to " << ref->m_path
              << ": images have different dimensions." << std::endl;
    return;
  }

  using namespace cv;
  Mat ref_gray;
  cvtColor(ref->image(), ref_gray, COLOR_BGR2GRAY);

  Mat to_align_gray;
  cvtColor(to_align->image(), to_align_gray, COLOR_BGR2GRAY);

  const auto warp_mode = MOTION_TRANSLATION;
  const int num_iterations = 20000;
  const double termination_eps = 1.0e-6;
  Mat warp_matrix = Mat::eye(2, 3, CV_32F);

  try {
    double correlation_val =
        findTransformECC(ref_gray, to_align_gray, warp_matrix, warp_mode,
                         TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
                                      num_iterations, termination_eps));

  } catch (cv::Exception &e) {
    std::cerr << "Could not align " << to_align->m_path << " to " << ref->m_path
              << ": " << e.what() << std::endl;
    return;
  }
  // Do the alignment.
  Mat warped_image =
      Mat(to_align->image().rows, to_align->image().cols, CV_32FC1);
  warpAffine(to_align->image(), warped_image, warp_matrix, warped_image.size(),
             INTER_LINEAR + WARP_INVERSE_MAP);
  to_align->update_image(warped_image);
}
} // namespace StackExposures
