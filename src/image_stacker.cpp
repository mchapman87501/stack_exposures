#include "image_stacker.hpp"
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace StackExposures {

namespace {
struct ImageStackerImpl : public ImageStacker {
  ImageStackerImpl() : m_width(0), m_height(0), m_image() {}

  void push(const cv::Mat &new_image) override {
    const size_t w_new(new_image.cols);
    const size_t h_new(new_image.rows);

    if ((m_width == 0) || (m_height == 0)) {
      m_width = w_new;
      m_height = h_new;
      m_image = cv::Mat(m_height, m_width, CV_32FC3);
    }
    // new_image must be the same size...
    if ((w_new == m_width) && (h_new == m_height)) {
      cv::Mat sum;
      cv::add(m_image, new_image, sum, cv::noArray(), CV_32FC3);
      m_image = sum;
    } else {
      std::cerr << "Cannot push image of (width x height) "
                << "(" << w_new << " x " << h_new << ") onto a stack of size "
                << "(" << m_width << " x " << m_height << ")." << std::endl;
    }
  }

  cv::Mat result8() const override {
    return converted(m_image, 255.0, CV_8UC3);
  }

  cv::Mat result16() const override {
    return converted(m_image, 65535.0, CV_16UC3);
  }

  cv::Mat resultf() const override { return converted(m_image, 1.0, CV_32FC3); }

private:
  size_t m_width;
  size_t m_height;
  cv::Mat m_image;

  std::pair<double, double> value_range(const cv::Mat &image) const {
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    double min_val = 0.0;
    double max_val = 0.0;
    cv::minMaxIdx(gray, &min_val, &max_val);
    return std::make_pair(min_val, max_val);
  }

  cv::Mat converted(const cv::Mat &image, double max_out,
                    auto cv_img_format) const {
    cv::Mat result;

    const auto v_extrema = value_range(image);
    const double min_val = v_extrema.first;
    const double max_val = v_extrema.second;

    const double dval = max_val - min_val;
    const double alpha = (dval > 0.0) ? (max_out / dval) : 1.0;
    // Beta offset is applied after scaling by alpha, hence multiplication by
    // alpha.
    const double beta = -min_val * alpha;
    image.convertTo(result, cv_img_format, alpha, beta);
    // No need for colorspace conversion.  Only data type conversion
    // (scaling, offset) was needed.
    return result;
  }
};

} // namespace

ImageStacker::Ptr ImageStacker::create() {
  return std::shared_ptr<ImageStackerImpl>(new ImageStackerImpl());
}

} // namespace StackExposures