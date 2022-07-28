#include "image_info.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <utility>

namespace StackExposures {

ImageInfo::SharedPtr ImageInfo::from_file(std::filesystem::path path,
                                          cv::Mat &image) {
  return std::shared_ptr<ImageInfo>(new ImageInfo(path, image));
}

ImageInfo::SharedPtr
ImageInfo::from_raw_file(LibRawSharedPtr processor, std::filesystem::path path,
                         libraw_processed_image_t *raw_img) {
  return std::shared_ptr<ImageInfo>(new ImageInfo(processor, path, raw_img));
}

ImageInfo::SharedPtr ImageInfo::with_image(const ImageInfo &src,
                                           cv::Mat &image) {
  return std::shared_ptr<ImageInfo>(new ImageInfo(src, image));
}

ImageInfo::ImageInfo(std::filesystem::path path, cv::Mat &image)
    : m_path(std::move(path)), m_raw_img(nullptr), m_image(image) {}

ImageInfo::ImageInfo(LibRawSharedPtr processor, std::filesystem::path path,
                     libraw_processed_image_t *raw_img)
    : m_path(std::move(path)),
      m_raw_img(LibRawProcessedImageSharedPtr(
          raw_img, [processor](auto p) { processor->dcraw_clear_mem(p); })) {
  int h = m_raw_img->height;
  int w = m_raw_img->width;

  // m_image references new_value->data but doesn't take ownership.
  // ImageInfo stores m_raw_img -- memory management.
  cv::Mat from_raw(h, w, CV_8UC3, (void *)m_raw_img->data);
  cv::cvtColor(from_raw, m_image, cv::COLOR_RGB2BGR);
}

ImageInfo::ImageInfo(const ImageInfo &src, cv::Mat &image)
    : m_path(src.m_path), m_raw_img(src.m_raw_img), m_image(image) {}

const std::filesystem::path &ImageInfo::path() const { return m_path; }

const cv::Mat &ImageInfo::image() const { return m_image; }

bool ImageInfo::same_extents(ImageInfo::SharedPtr other_info) const {
  const cv::Mat &other(other_info->image());
  return ((m_image.rows == other.rows) && (m_image.cols == other.cols));
}

size_t ImageInfo::rows() const { return m_image.rows; }

size_t ImageInfo::cols() const { return m_image.cols; }

bool ImageInfo::empty() const { return m_image.empty(); }

} // namespace StackExposures