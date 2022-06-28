#include "image_info.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>

namespace StackExposures {
ImageInfo::ImageInfo(LibRawPtr processor, const std::filesystem::path &path,
                     libraw_processed_image_t *raw_img)
    : m_processor(processor), m_path(path), m_raw_img(raw_img) {
  int h = m_raw_img->height;
  int w = m_raw_img->width;
  int c = m_raw_img->colors;

  // m_image references new_value->data but doesn't take ownership.
  cv::Mat from_raw(h, w, CV_8UC3, (void *)m_raw_img->data);
  cv::cvtColor(from_raw, m_image, cv::COLOR_RGB2BGR);
}

ImageInfo::~ImageInfo() {
  if (m_raw_img) {
    m_processor->dcraw_clear_mem(m_raw_img);
    m_raw_img = nullptr;
  }
}
} // namespace StackExposures