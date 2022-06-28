#pragma once

#include <filesystem>
#include <memory>

#include "image_info.hpp"
#include "shared_ptrs.hpp"

namespace StackExposures {
class AlignedImageGenerator {
public:
  using UniquePtr = std::unique_ptr<AlignedImageGenerator>;

  AlignedImageGenerator();

  /**
   * @brief Align an(other) image.
   *
   * Aligns the image at 'image_path' with all images aligned so far.
   * If no images have been aligned, this becomes the reference image
   * against which all subsequent calls to 'align' will align.
   *
   * @param[in] image_path the path to the image (including raw) to align
   *
   * @return ImageInfo for the aligned image.
   */

  ImageInfo::Ptr align(const std::filesystem::path &image_path);

protected:
  LibRawPtr m_processor;

  ImageInfo::Ptr m_ref_img = nullptr;

  ImageInfo::Ptr load_image(const std::filesystem::path &image_path);
  [[nodiscard]] ImageInfo::Ptr align_internal(ImageInfo::Ptr ref,
                                              ImageInfo::Ptr unaligned);
};
} // namespace StackExposures
