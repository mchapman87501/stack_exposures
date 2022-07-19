#include "mean_image_stacker.hpp"
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace StackExposures {

namespace {
struct MeanImageStackerImpl : public MeanImageStacker {
  MeanImageStackerImpl()
      : m_width(0), m_height(0), m_count(0), m_image(), m_dark_image() {}

  void add(const cv::Mat &new_image) override {
    if ((m_width == 0) && (m_height == 0)) {
      m_width = new_image.cols;
      m_height = new_image.rows;
      m_image = cv::Mat(m_height, m_width, CV_32FC3, cv::Scalar(0, 0, 0));
    }
    // new_image must be the same size.
    if (check_size(new_image, "image")) {
      cv::Mat sum;
      cv::add(m_image, new_image, sum, cv::noArray(), CV_32FC3);
      m_image = sum;

      m_count += 1;
    }
  }

  void subtract(const cv::Mat &new_image) override {
    if (m_dark_image.empty()) {
      new_image.convertTo(m_dark_image, CV_32FC3);
    }
  }

  cv::Mat result8() const override { return converted(0xFF, CV_8UC3); }

  cv::Mat result16() const override { return converted(0xFFFF, CV_16UC3); }

private:
  size_t m_width;
  size_t m_height;

  size_t m_count;

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

  cv::Mat converted(double max_out, int cv_img_format) const {
    if (0 == m_count) {
      // No images were added.
      return m_image;
    }

    cv::Mat darkened;
    subtract_dark_image(m_image, darkened);

    cv::Mat result;
    const double alpha = 1.0 / m_count;
    darkened.convertTo(result, cv_img_format, alpha);

    // No need for colorspace conversion.  Only data type conversion
    // (scaling, offset) was needed.
    return result;
  }

  void subtract_dark_image(const cv::Mat &src, cv::Mat &result) const {
    if (!m_dark_image.empty() && check_size(m_dark_image, "dark image")) {
      result = src - m_count * m_dark_image;
    } else {
      result = src;
    }
  }
};

} // namespace

MeanImageStacker::Ptr MeanImageStacker::create() {
  return std::shared_ptr<MeanImageStackerImpl>(new MeanImageStackerImpl());
}

} // namespace StackExposures