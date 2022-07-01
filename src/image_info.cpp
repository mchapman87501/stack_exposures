#include "image_info.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>

namespace StackExposures {

ImageInfo::ImageInfo(const std::filesystem::path &path, cv::Mat &image)
    : m_path(path), m_raw_img(nullptr), m_image(image) {}

ImageInfo::ImageInfo(LibRawPtr processor, const std::filesystem::path &path,
                     libraw_processed_image_t *raw_img)
    : m_path(path),
      m_raw_img(LibRawProcessedImagePtr(
          raw_img, [processor](auto p) { processor->dcraw_clear_mem(p); })) {
  int h = m_raw_img->height;
  int w = m_raw_img->width;
  int c = m_raw_img->colors;

  // m_image references new_value->data but doesn't take ownership.
  // ImageInfo stores m_raw_img -- memory management.
  cv::Mat from_raw(h, w, CV_8UC3, (void *)m_raw_img->data);
  cv::cvtColor(from_raw, m_image, cv::COLOR_RGB2BGR);
}

ImageInfo::ImageInfo(const ImageInfo &src, cv::Mat &image)
    : m_path(src.m_path), m_raw_img(src.m_raw_img), m_image(image) {}

const std::filesystem::path &ImageInfo::path() const { return m_path; }

const cv::Mat &ImageInfo::image() const { return m_image; }

bool ImageInfo::same_extents(ImageInfo::Ptr other_info) const {
  const cv::Mat &other(other_info->image());
  return ((m_image.rows == other.rows) && (m_image.cols == other.cols));
}

size_t ImageInfo::rows() const { return m_image.rows; }

size_t ImageInfo::cols() const { return m_image.cols; }

} // namespace StackExposures