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
  using namespace cv;
  // normalize and convert to 8-bit unsigned.
  Mat result;

  // From https://stackoverflow.com/a/26409969/2826337
  double min_val = 0.0;
  double max_val = 0.0;
  Mat one_channel = m_image.reshape(1);
  minMaxIdx(one_channel, &min_val, &max_val);

  const double dval = max_val - min_val;
  const double alpha = (dval > 0.0) ? (255.0 / dval) : 1.0;
  // Beta offset is applied after scaling by alpha, hence multiplication by
  // alpha.
  const double beta = -min_val * alpha;
  m_image.convertTo(result, CV_8UC3, alpha, beta);
  // No need for colorspace conversion.  Only data type conversion
  // (scaling, offset) was needed.
  return result;
}

} // namespace StackExposures