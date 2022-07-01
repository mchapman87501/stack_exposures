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
  // ImageInfo stores m_raw_img -- memory management.
  cv::Mat from_raw(h, w, CV_8UC3, (void *)m_raw_img->data);
  cv::cvtColor(from_raw, m_image, cv::COLOR_RGB2BGR);
}

ImageInfo::ImageInfo(const ImageInfo &src, cv::Mat &image)
    : m_path(src.m_path), m_processor(src.m_processor),
      m_raw_img(src.m_raw_img), m_image(image) {}

ImageInfo::~ImageInfo() {
  if (m_raw_img) {
    m_processor->dcraw_clear_mem(m_raw_img.get());
  }
}

const std::filesystem::path &ImageInfo::path() const { return m_path; }

cv::Mat &ImageInfo::image() { return m_image; }

bool ImageInfo::same_extents(ImageInfo::Ptr other_info) const {
  const cv::Mat &other(other_info->image());
  return ((m_image.rows == other.rows) && (m_image.cols == other.cols));
}

size_t ImageInfo::rows() const { return m_image.rows; }

size_t ImageInfo::cols() const { return m_image.cols; }

} // namespace StackExposures