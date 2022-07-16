#include "image_loader.hpp"

#include <iostream>

#include <opencv2/imgcodecs.hpp>

namespace StackExposures {
namespace {
void configure(LibRawPtr processor) {
  // Adjust processing of raw images.
  // NB: don't muck with bit depth.  Load with default 8-bits / color component,
  // to match cv::imread defaults.

  processor->imgdata.params.use_camera_wb = 1;

  // Use sRGB color space and gamma curve.  See output_color, and See gamm[6],
  // at https://www.libraw.org/docs/API-datastruct.html#libraw_output_params_t
  processor->imgdata.params.output_color = 1; // sRGB
  processor->imgdata.params.gamm[0] = 1.0 / 2.4;
  processor->imgdata.params.gamm[1] = 12.92;
  processor->imgdata.params.no_auto_bright = 1;

  // TODO - ARW-specific parameters, e.g., to suppress posterization
  // in shadows of Sony RAW images.
  // See https://www.libraw.org/docs/API-datastruct.html#libraw_output_params_t
  // and search for sony_arw2_posterization_thr
}

} // namespace

ImageLoader::ImageLoader() : m_processor(std::make_shared<LibRaw>()) {
  configure(m_processor);
}

void ImageLoader::check(int status, const std::string &msg) {
  using namespace std;

  if (LIBRAW_FATAL_ERROR(status)) {
    ostringstream outs;
    outs << "Fatal error: " << msg << "; status = " << status << " ("
         << m_processor->strerror(status) << ")";
    cerr << outs.str() << endl;
    throw runtime_error(outs.str());
  }
  if (LIBRAW_SUCCESS != status) {
    const auto err =
        (status < 0) ? m_processor->strerror(status) : std::strerror(status);
    cerr << msg << "; status = " << status << " (" << err << ")" << endl;
    throw runtime_error(msg);
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
  // Consider adjusting m_processor differently when it appears that a Sony ARW
  // image is being loaded.
  check(m_processor->open_file(image_path.c_str()), "Could not open file");
  check(m_processor->unpack(), "Could not unpack");
  check(m_processor->dcraw_process(),
        "dcraw_process"); // This is what Rawpy uses.

  // Can LibRaw do the right thing with raw images having > 10 bits / channel?
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