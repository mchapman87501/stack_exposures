#include "image_stacker.hpp"

namespace StackExposures {
ImageStacker::ImageStacker() : m_width(0), m_height(0) {}

void ImageStacker::push(const cv::Mat &new_image) {
  using namespace cv;

  const size_t w_new(new_image.cols);
  const size_t h_new(new_image.rows);

  if ((m_width == 0) || (m_height == 0)) {
    m_width = w_new;
    m_height = h_new;
    // m_image needs to store floating point values
    // in order to accumulate intensities from multiple images.
    m_image = Mat(m_height, m_width, CV_64FC3);
  }
  // new_image must be the same size...
  if ((w_new == m_width) && (h_new == m_height)) {
    cv::Mat sum(m_image);
    cv::add(m_image, new_image, sum, cv::noArray(), CV_64FC3);
    m_image = sum;
  }
}

cv::Mat ImageStacker::result() {
  // TODO normalize and convert to BGR.
  return m_image;
}

} // namespace StackExposures