#include "mean_image_stacker.hpp"
#include <iomanip>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace StackExposures {

namespace {
struct MeanImageStackerImpl : public MeanImageStacker {
  MeanImageStackerImpl() : m_width(0), m_height(0), m_count(0), m_image() {}

  void push(const cv::Mat &new_image) override {
    const size_t w_new(new_image.cols);
    const size_t h_new(new_image.rows);

    if ((m_width == 0) || (m_height == 0)) {
      m_width = w_new;
      m_height = h_new;
      m_image = cv::Mat(m_height, m_width, CV_32FC3);
    }
    // new_image must be the same size.
    if ((w_new == m_width) && (h_new == m_height)) {
      cv::Mat sum;
      cv::add(m_image, normed(new_image), sum, cv::noArray(), CV_32FC3);
      m_image = sum;

      m_count += 1;
    } else {
      std::cerr << "Cannot push image of (width x height) "
                << "(" << w_new << " x " << h_new << ") onto a stack of size "
                << "(" << m_width << " x " << m_height << ")." << std::endl;
    }
  }

  cv::Mat result8() const override { return converted(m_image, 0xFF, CV_8UC3); }

  cv::Mat result16() const override {
    return converted(m_image, 0xFFFF, CV_16UC3);
  }

protected:
  // TODO extract these to a distinct class to ease unit testing.

  // Get image, normed to 0..1.0
  cv::Mat normed(const cv::Mat &image) {
    // This derives from OpenCV's internal convertToShow.
    const int depth = image.depth();
    if (depth == CV_32F) {
      return image;
    }
    cv::Mat result;
    const double scale = norm_scale(depth);
    const double offset = norm_offset(depth);
    image.convertTo(result, CV_32FC3, scale, offset);
    return result;
  }

  double norm_scale(int depth) {
    switch (depth) {
    case CV_8U:
      return 1.0 / double(0xFF);
    }
    std::cerr << "DEBUG: Input is unknown bit-depth" << std::endl;
    return 1.0;
  }

  double norm_offset(int depth) {
    if ((depth == CV_8S) || (depth == CV_16S)) {
      return 0.5;
    }
    return 0.0;
  }

private:
  size_t m_width;
  size_t m_height;
  cv::Mat m_image;
  size_t m_count;

  cv::Mat converted(const cv::Mat &image, double max_out,
                    auto cv_img_format) const {
    cv::Mat result;

    // Stretch 0..1 to 0..max_out.
    const double alpha = max_out / m_count;
    image.convertTo(result, cv_img_format, alpha);

    // No need for colorspace conversion.  Only data type conversion
    // (scaling, offset) was needed.
    return result;
  }
};

} // namespace

MeanImageStacker::Ptr MeanImageStacker::create() {
  return std::shared_ptr<MeanImageStackerImpl>(new MeanImageStackerImpl());
}

} // namespace StackExposures