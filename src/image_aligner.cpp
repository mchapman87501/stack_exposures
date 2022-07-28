#include "image_aligner.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

namespace {
using namespace StackExposures;

} // namespace

namespace StackExposures {

ImageInfo::SharedPtr ImageAligner::align(ImageInfo::SharedPtr image) {
  if (!m_ref_img) {
    m_ref_img = image;
    return image;
  }

  if (!m_ref_img->same_extents(image)) {
    std::cerr << "Cannot align " << image->path() << " to " << m_ref_img->path()
              << ": images have different dimensions." << std::endl;
  } else {
    try {
      // Align any subsequent image to this most recent alignment.
      m_ref_img = align_internal(m_ref_img, image);
      return m_ref_img;
    } catch (cv::Exception &e) {
      std::cerr << "Could not align " << image->path() << " to "
                << m_ref_img->path() << ": " << e.what() << std::endl;
    }
  }

  return nullptr;
}

ImageInfo::SharedPtr
ImageAligner::align_internal(ImageInfo::SharedPtr ref,
                             ImageInfo::SharedPtr to_align) {
  // See
  // https://docs.opencv.org/4.6.0/dd/d93/samples_2cpp_2image_alignment_8cpp-example.html#a39

  using namespace cv;

  std::cout << "  Create alignment grayscales." << std::endl;
  Mat ref_gray;
  cvtColor(ref->image(), ref_gray, COLOR_BGR2GRAY);

  Mat to_align_gray;
  cvtColor(to_align->image(), to_align_gray, COLOR_BGR2GRAY);

  const auto warp_mode = MOTION_EUCLIDEAN;
  const int num_iterations = 200;
  const double termination_eps = 1.0e-4;
  Mat warp_matrix = Mat::eye(2, 3, CV_32F);

  std::cout << "  Find transformECC" << std::endl;
  findTransformECC(ref_gray, to_align_gray, warp_matrix, warp_mode,
                   TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
                                num_iterations, termination_eps));

  std::cout << "  Align" << std::endl;
  // Do the alignment.
  Mat warped_image =
      Mat(to_align->image().rows, to_align->image().cols, CV_32FC1);
  warpAffine(to_align->image(), warped_image, warp_matrix, warped_image.size(),
             INTER_LINEAR + WARP_INVERSE_MAP);
  std::cout << "  Return aligned result" << std::endl;
  return ImageInfo::with_image(*to_align, warped_image);
}
} // namespace StackExposures
