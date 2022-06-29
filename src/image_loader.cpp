#include "image_loader.hpp"

#include <iostream>

#include <opencv2/imgcodecs.hpp>

namespace StackExposures {
ImageLoader::ImageLoader() : m_processor(std::make_shared<LibRaw>()) {}

void ImageLoader::check(int status, const std::string &msg) {
  using namespace std;

  if (LIBRAW_FATAL_ERROR(status)) {
    ostringstream outs;
    outs << "Fatal error: " << msg << "; status = " << status << " ("
         << m_processor->strerror(status) << ")";
    cerr << outs.str() << endl;
    throw new runtime_error(outs.str());
  }
  if (LIBRAW_SUCCESS != status) {
    const char *status_msg =
        (status < 0) ? m_processor->strerror(status) : std::strerror(status);
    cerr << msg << "; status = " << status << " (" << status_msg << ")" << endl;
    throw new runtime_error(msg);
  }
}

ImageInfo::Ptr
ImageLoader::load_image(const std::filesystem::path &image_path) {
  cv::Mat image = cv::imread(image_path.c_str());
  if (image.data != nullptr) {
    return std::make_shared<ImageInfo>(image_path, image);
  }
  return load_raw_image(image_path);
}

ImageInfo::Ptr
ImageLoader::load_raw_image(const std::filesystem::path &image_path) {
  check(m_processor->open_file(image_path.c_str()), "Could not open file");
  check(m_processor->unpack(), "Could not unpack");

  check(m_processor->dcraw_process(),
        "dcraw_process"); // This is what Rawpy uses.

  // To extract image data, rawpy uses
  // dcraw_make_mem_image().
  int status = 0;
  libraw_processed_image_t *img = m_processor->dcraw_make_mem_image(&status);
  check(status, "dcraw_make_mem_image");
  assert(img);
  assert(img->type == LIBRAW_IMAGE_BITMAP);

  ImageInfo::Ptr result =
      std::make_shared<ImageInfo>(m_processor, image_path, img);
  return result;
}

} // namespace StackExposures