#pragma once

#include <memory>

#include "image_info.hpp"

#include "libraw.h"

namespace StackExposures {
class ImageLoader {
public:
  ImageLoader();

  /**
   * @brief      Loads an image.
   *
   * @param[in]  image_path  pathname of the image
   *
   * @return     The loaded image, with metadata.  The 'image()' of the return
   * value may be empty in case of error.
   */
  ImageInfo::SharedPtr load_image(const std::filesystem::path &image_path);

private:
  LibRawSharedPtr m_processor;

  void check(int status, const std::string &msg);

  ImageInfo::SharedPtr load_raw_image(const std::filesystem::path &image_path);
};
} // namespace StackExposures