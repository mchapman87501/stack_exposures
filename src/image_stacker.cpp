#include "image_stacker.hpp"
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace StackExposures {

namespace {
struct ImageStackerImpl : public ImageStacker {
  ImageStackerImpl() : m_width(0), m_height(0), m_image(), m_dark_image() {}

  void add(const cv::Mat &new_image) override {
    const size_t w_new(new_image.cols);
    const size_t h_new(new_image.rows);

    if ((m_width == 0) || (m_height == 0)) {
      m_width = w_new;
      m_height = h_new;
      m_image = cv::Mat(m_height, m_width, CV_32FC3);
    }
    // new_image must be the same size...
    if (check_size(new_image, "image")) {
      cv::Mat sum;
      cv::add(m_image, new_image, sum, cv::noArray(), CV_32FC3);
      m_image = sum;
    }
  }

  void subtract(const cv::Mat &new_image) override {
    // Should this be a last-one-wins?
    if (m_dark_image.empty()) {
      new_image.convertTo(m_dark_image, CV_32FC3);
    }
  }

  cv::Mat result8() const override {
    return converted(m_image, 255.0, CV_8UC3);
  }

  cv::Mat result16() const override {
    return converted(m_image, 65535.0, CV_16UC3);
  }

private:
  size_t m_width;
  size_t m_height;
  cv::Mat m_image;

  cv::Mat m_dark_image;

  bool check_size(const cv::Mat &new_image, std::string_view descr) const {
    if ((new_image.cols != m_width) || (new_image.rows != m_height)) {
      std::cerr << descr << " (width x height) "
                << "(" << new_image.cols << " x " << new_image.rows
                << ")  is incompatible with established size "
                << "(" << m_width << " x " << m_height << ")." << std::endl;
      return false;
    }
    return true;
  }

  std::pair<double, double> value_range(const cv::Mat &image) const {
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    double min_val = 0.0;
    double max_val = 0.0;
    cv::minMaxIdx(gray, &min_val, &max_val);
    return std::make_pair(min_val, max_val);
  }

  cv::Mat converted(const cv::Mat &image, double max_out,
                    int cv_img_format) const {
    cv::Mat darkened = subtracting_dark_image(image);

    const auto v_extrema = value_range(darkened);
    const double min_val = v_extrema.first;
    const double max_val = v_extrema.second;

    const double dval = max_val - min_val;
    const double alpha = (dval > 0.0) ? (max_out / dval) : 1.0;

    cv::Mat result;
    // Beta offset is applied after scaling by alpha, hence multiplication by
    // alpha.
    const double beta = -min_val * alpha;
    darkened.convertTo(result, cv_img_format, alpha, beta);
    return result;
  }

  cv::Mat subtracting_dark_image(const cv::Mat &src) const {
    if (!m_dark_image.empty() && check_size(m_dark_image, "dark image")) {
      cv::Mat result;
      cv::subtract(src, m_dark_image, result);
      return result;
    }
    return src;
  }
};

} // namespace

ImageStacker::Ptr ImageStacker::create() {
  return std::shared_ptr<ImageStackerImpl>(new ImageStackerImpl());
}

} // namespace StackExposures