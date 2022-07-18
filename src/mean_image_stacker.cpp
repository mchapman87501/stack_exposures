#include "mean_image_stacker.hpp"
#include <iomanip>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace StackExposures {

namespace {
struct MeanImageStackerImpl : public MeanImageStacker {
  MeanImageStackerImpl()
      : m_width(0), m_height(0), m_count(0), m_image(), m_dark_image() {}

  void add(const cv::Mat &new_image) override {
    const size_t w_new(new_image.cols);
    const size_t h_new(new_image.rows);

    if ((m_width == 0) || (m_height == 0)) {
      m_width = w_new;
      m_height = h_new;
      m_image = cv::Mat(m_height, m_width, CV_32FC3);
    }
    // new_image must be the same size.
    if (check_size(new_image, "image")) {
      cv::Mat sum;
      cv::add(m_image, normed(new_image), sum, cv::noArray(), CV_32FC3);
      m_image = sum;

      m_count += 1;
    }
  }

  void subtract(const cv::Mat &new_image) override {
    // Should this be a last-one-wins?
    if (m_dark_image.empty()) {
      m_dark_image = new_image;
    }
  }

  cv::Mat result8() const override { return converted(m_image, 0xFF, CV_8UC3); }

  cv::Mat result16() const override {
    return converted(m_image, 0xFFFF, CV_16UC3);
  }

protected:
  // TODO extract these to a distinct class to ease unit testing.

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

  cv::Mat subtracting_dark_image(const cv::Mat &src) const {
    if (!m_dark_image.empty() && check_size(m_dark_image, "dark image")) {
      cv::Mat result;
      cv::subtract(src, normed(m_dark_image), result);
      return result;
    }
    return src;
  }

  // Get image, normed to 0..1.0
  cv::Mat normed(const cv::Mat &image) const {
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

  double norm_scale(int depth) const {
    switch (depth) {
    case CV_8U:
      return 1.0 / double(0xFF);
    }
    std::cerr << "DEBUG: Input is unknown bit-depth" << std::endl;
    return 1.0;
  }

  double norm_offset(int depth) const {
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

  cv::Mat m_dark_image;

  cv::Mat converted(const cv::Mat &image, double max_out,
                    int cv_img_format) const {
    cv::Mat darkened = subtracting_dark_image(image);

    // Stretch 0..1 to 0..max_out.
    cv::Mat result;
    const double alpha = max_out / m_count;
    darkened.convertTo(result, cv_img_format, alpha);

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