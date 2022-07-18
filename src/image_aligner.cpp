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

ImageInfo::Ptr ImageAligner::align(ImageInfo::Ptr image) {
  if (!m_ref_img) {
    m_ref_img = image;
  } else {
    if (!m_ref_img->same_extents(image)) {
      std::cerr << "Cannot align " << image->path() << " to "
                << m_ref_img->path() << ": images have different dimensions."
                << std::endl;
    } else {
      try {
        return align_internal(m_ref_img, image);
      } catch (cv::Exception &e) {
        std::cerr << "Could not align " << image->path() << " to "
                  << m_ref_img->path() << ": " << e.what() << std::endl;
      }
    }
  }
  return image;
}

ImageInfo::Ptr ImageAligner::align_internal(ImageInfo::Ptr ref,
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
  const double termination_eps = 1.0e-6;
  Mat warp_matrix = Mat::eye(2, 3, CV_32F);

  findTransformECC(ref_gray, to_align_gray, warp_matrix, warp_mode,
                   TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
                                num_iterations, termination_eps));

  // Do the alignment.
  Mat warped_image =
      Mat(to_align->image().rows, to_align->image().cols, CV_32FC1);
  warpAffine(to_align->image(), warped_image, warp_matrix, warped_image.size(),
             INTER_LINEAR + WARP_INVERSE_MAP);

  return std::make_shared<ImageInfo>(*to_align, warped_image);
}
} // namespace StackExposures
