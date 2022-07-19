#include "image_stacker.hpp"
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace StackExposures {

namespace {
struct ImageStackerImpl : public ImageStacker {
  ImageStackerImpl()
      : m_width(0), m_height(0), m_count(0), m_image(), m_dark_image() {}

  void add(const cv::Mat &new_image) override {
    if ((m_width == 0) && (m_height == 0)) {
      m_width = new_image.cols;
      m_height = new_image.rows;
      m_image = cv::Mat(m_height, m_width, CV_32FC3, cv::Scalar(0, 0, 0));
    }
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
    // Goal: Rescale the summed BGR values so that the corresponding
    // hue and saturation are unchanged, but the value extends across 0...Vmax.

    if (0 == m_count) {
      // No images were added.
      return m_image;
    }

    cv::Mat darkened;
    subtract_dark_image(m_image, darkened);

    cv::Mat mean_bgr;
    darkened.convertTo(mean_bgr, darkened.type(), 1.0 / double(m_count));

    cv::Mat mean_hsv;
    cv::cvtColor(mean_bgr, mean_hsv, cv::COLOR_BGR2HSV);

    cv::Mat stretched_hsv;
    stretch_hsv_channel(mean_hsv, stretched_hsv);

    cv::Mat stretched_bgr;
    cv::cvtColor(stretched_hsv, stretched_bgr, cv::COLOR_HSV2BGR);

    cv::Mat result;
    stretched_bgr.convertTo(result, cv_img_format);
    return result;
  }

  void subtract_dark_image(const cv::Mat &src, cv::Mat &result) const {
    if (!m_dark_image.empty() && check_size(m_dark_image, "dark image")) {
      // Assume each stacked image has the same noise pattern, represented
      // by the dark image.
      result = src - m_count * m_dark_image;
    } else {
      result = src;
    }
  }

  void stretch_hsv_channel(const cv::Mat &hsv, cv::Mat &result) const {
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);

    double min_val = 0.0;
    double max_val = 0.0;
    cv::minMaxIdx(channels[2], &min_val, &max_val);

    // If all values are the same, do not adjust.
    if (max_val > min_val) {
      // I don't know the range of V values for CV_32FC3.
      // The OpenCV documentation suggests it should be 0..1, but that
      // appears to be untrue for my images.  Likely reason: they
      // are CV_32FC3, accumulating (presumably) 8-bit components.
      constexpr double out_max = 255.0;
      const double alpha = out_max / (max_val - min_val);
      // Beta needs to be prescaled by alpha.
      const double beta = -min_val * alpha;

      cv::Mat new_v_chan;
      channels[2].convertTo(new_v_chan, channels[2].type(), alpha, beta);
      channels[2] = new_v_chan;
    }

    cv::merge(channels, result);
  }
};

} // namespace

ImageStacker::Ptr ImageStacker::create() {
  return std::shared_ptr<ImageStackerImpl>(new ImageStackerImpl());
}

} // namespace StackExposures