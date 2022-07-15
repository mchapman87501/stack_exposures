#pragma once

#include <memory>

#include "image_info.hpp"

#include "libraw.h"

namespace StackExposures {
class ImageLoader {
public:
  using Ptr = std::shared_ptr<ImageLoader>;
  static Ptr create() { return Ptr(new ImageLoader()); }

  /**
   * @brief      Loads an image.
   *
   * @param[in]  image_path  pathname of the image
   *
   * @return     The loaded image, with metadata.  The 'image()' of the return
   * value may be empty in case of error.
   */
  ImageInfo::Ptr load_image(const std::filesystem::path &image_path);

private:
  LibRawPtr m_processor;

  ImageLoader();

  void check(int status, const std::string &msg);

  ImageInfo::Ptr load_raw_image(const std::filesystem::path &image_path);
};
} // namespace StackExposures