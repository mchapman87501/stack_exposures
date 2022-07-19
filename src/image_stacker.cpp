#include "image_stacker.hpp"
#include <iostream>

#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace StackExposures {

namespace {

struct Mean : public IImageStacker {
  Mean() : m_image(), m_dark_image() {}

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

  [[nodiscard]] cv::Mat result8() const override {
    return converted(1, CV_8UC3);
  }

  [[nodiscard]] cv::Mat result16() const override {
    return converted(0xFF, CV_16UC3);
  }

protected:
  [[nodiscard]] cv::Mat virtual converted(size_t scale,
                                          int cv_img_format) const {
    if (empty()) {
      // No images were added.
      return {};
    }

    cv::Mat rescaled = darkened() * scale;

    // Convert from CV_32FC3 to cv_img_format.
    cv::Mat result;
    rescaled.convertTo(result, cv_img_format);

    return result;
  }

  [[nodiscard]] bool empty() const { return m_count == 0; }

  [[nodiscard]] cv::Mat darkened() const {
    cv::Mat mean = m_image / m_count;

    if (!m_dark_image.empty() && check_size(m_dark_image, "dark image")) {
      // NB this can result in negative pixel values.
      // Rely on convertTo to clip to 0.
      return mean - m_dark_image;
    }
    return mean;
  }

private:
  size_t m_width{0};
  size_t m_height{0};

  size_t m_count{0};

  cv::Mat m_image;
  cv::Mat m_dark_image;

  [[nodiscard]] bool check_size(const cv::Mat &new_image,
                                std::string_view descr) const {
    if ((new_image.cols != m_width) || (new_image.rows != m_height)) {
      std::cerr << descr << " (width x height) "
                << "(" << new_image.cols << " x " << new_image.rows
                << ")  is incompatible with established size "
                << "(" << m_width << " x " << m_height << ")." << std::endl;
      return false;
    }
    return true;
  }
};

struct Stretch : public Mean {
protected:
  [[nodiscard]] cv::Mat converted(size_t scale,
                                  int cv_img_format) const override {
    // Goal: Rescale the summed BGR values so that the corresponding
    // hue and saturation are unchanged, but the value extends across 0...Vmax.

    if (empty()) {
      return {};
    }

    cv::Mat mean_hsv;
    cv::cvtColor(darkened(), mean_hsv, cv::COLOR_BGR2HSV);

    cv::Mat stretched_hsv;
    stretch_hsv_channel(mean_hsv, stretched_hsv);

    cv::Mat stretched_bgr;
    cv::cvtColor(stretched_hsv, stretched_bgr, cv::COLOR_HSV2BGR);

    cv::Mat rescaled = stretched_bgr * scale;

    // Convert from CV_32FC3 to cv_img_format.
    cv::Mat result;
    rescaled.convertTo(result, cv_img_format);
    return result;
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
      cv::Mat new_v_chan =
          out_max * (channels[2] - min_val) / (max_val - min_val);
      channels[2] = new_v_chan;
      cv::merge(channels, result);
    } else {
      result = hsv;
    }
  }
};

} // namespace

namespace ImageStacker {
IImageStacker::Ptr mean() { return std::make_unique<Mean>(); }
IImageStacker::Ptr stretch() { return std::make_unique<Stretch>(); }
} // namespace ImageStacker

} // namespace StackExposures