#include "image_info.hpp"

namespace StackExposures {
ImageInfo::~ImageInfo() {
  if (m_raw_img) {
    m_processor.dcraw_clear_mem(m_raw_img);
  }
}

void ImageInfo::set_raw_image(libraw_processed_image_t *new_value) {
  if (m_raw_img) {
    m_processor.dcraw_clear_mem(m_raw_img);
  }
  m_raw_img = new_value;

  // It looks as though the numpy array is just getting a block of
  // data straight from img.data.
  int h = new_value->height;
  int w = new_value->width;
  int c = new_value->colors;

  m_image = cv::Mat(h, w, CV_8SC3, (void *)new_value->data);
}

} // namespace StackExposures