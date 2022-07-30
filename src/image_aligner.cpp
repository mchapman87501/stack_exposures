#include "image_aligner.hpp"

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

namespace {
void align_internal(const cv::Mat &ref, const cv::Mat &to_align,
                    cv::Mat &aligned) {
  // See
  // https://docs.opencv.org/4.6.0/dd/d93/samples_2cpp_2image_alignment_8cpp-example.html#a39

  using namespace cv;

  Mat ref_gray;
  cvtColor(ref, ref_gray, COLOR_BGR2GRAY);

  Mat to_align_gray;
  cvtColor(to_align, to_align_gray, COLOR_BGR2GRAY);

  const auto warp_mode = MOTION_EUCLIDEAN;
  const int num_iterations = 200;
  const double termination_eps = 1.0e-5;
  Mat warp_matrix = Mat::eye(2, 3, CV_32F);

  findTransformECC(ref_gray, to_align_gray, warp_matrix, warp_mode,
                   TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
                                num_iterations, termination_eps));
  // Do the alignment.
  aligned = Mat(to_align.rows, to_align.cols, CV_32FC3);
  warpAffine(to_align, aligned, warp_matrix, aligned.size(),
             INTER_LINEAR + WARP_INVERSE_MAP);
}

} // namespace

void ImageAligner::align(const cv::Mat &ref, const cv::Mat &to_align,
                         cv::Mat &aligned) {

  if ((ref.cols != to_align.cols) || (ref.rows != to_align.rows)) {
    std::cerr << "Cannot align images with different sizes." << std::endl;
    aligned = cv::Mat();
  } else {
    try {
      align_internal(ref, to_align, aligned);
    } catch (cv::Exception &e) {
      std::cerr << "Could not align images: " << e.what() << std::endl;
      cv::imwrite("align_failed_ref.tiff", ref);
      cv::imwrite("align_failed_to_align.tiff", to_align);
      aligned = cv::Mat();
    }
  }
}

} // namespace StackExposures
