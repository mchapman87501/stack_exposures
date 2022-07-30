#include "image_stacker.hpp"

namespace StackExposures {
namespace {

constexpr auto image_dtype = CV_32FC3;

[[nodiscard]] cv::Mat stackable(const cv::Mat &image) {
  if (image.type() == image_dtype) {
    return image;
  }

  cv::Mat result;
  image.convertTo(result, image_dtype);
  return result;
}

struct StackedImage {
  cv::Mat m_image;
  size_t m_num_used{0}; // number of images used to compute m_image

  StackedImage() {}

  StackedImage(const cv::Mat &image, size_t num_used)
      : m_image(stackable(image)), m_num_used(num_used) {}

  StackedImage(const cv::Mat &single_image)
      : m_image(stackable(single_image)), m_num_used(1) {}

  bool succeeded() const { return (m_num_used > 0) && !m_image.empty(); }
  bool empty() const { return m_image.empty(); }
  bool same_extents(const StackedImage &other) const {
    return (m_image.rows == other.m_image.rows) &&
           (m_image.cols == other.m_image.cols);
  }
};

struct Impl : public ImageStacker {

  Impl() {}

  [[nodiscard]] cv::Mat stacked_result(const ImageInfoFutureContainer &images,
                                       ImageInfo::SharedPtr dark_image,
                                       bool align) const override {
    auto result = process_all(images, align);
    if (result.succeeded()) {
      cv::Mat mean = result.m_image / result.m_num_used;
      if (dark_image != nullptr) {
        return darkened(mean, dark_image->image());
      }
      return mean;
    }
    return cv::Mat(0, 0, image_dtype);
  }

private:
  void report_size_mismatch(ImageInfo::SharedPtr ref_img,
                            ImageInfo::SharedPtr img_info) const {
    report_size_mismatch(ref_img->image(), img_info->image(),
                         img_info->path().string());
  }

  void report_size_mismatch(const cv::Mat &ref_image, const cv::Mat &image,
                            std::string_view image_name) const {
    std::cerr << "Cannot process " << image_name.data()
              << ": image width x height (" << image.cols << " x " << image.rows
              << ") do not match first image (" << ref_image.cols << " x "
              << ref_image.rows << ")" << std::endl;
  }

  void report_empty() const {
    std::cerr << "Cannot process empty image." << std::endl;
  }

  [[nodiscard]] cv::Mat darkened(const cv::Mat &image,
                                 const cv::Mat &dark_image) const {
    const auto dark = stackable(dark_image);
    if ((image.rows == dark.rows) && (image.cols == dark.cols)) {
      auto result(image - dark);
      return result;
    }
    return image;
  }

  StackedImage process_all(const ImageInfoFutureContainer &images,
                           bool align) const {
    const auto count = images.size();

    if (count < 3) {
      if (count < 1) {
        std::cerr << "Can't align and stack -- need at least one image."
                  << std::endl;
        return {};
      } else if (count == 1) {
        return {images[0].get()->image()};
      }
      StackedImage s1(images[0].get()->image());
      StackedImage s2(images[1].get()->image());
      return align_and_stack(s1, s2, align);
    }

    // Alas, clang 14 doesn't support std::views.
    // TODO write tests to verify that all images are processed.
    const auto left_count = count / 2;
    const auto right_count = count - left_count;

    const auto left_result =
        process_some(images.begin(), images.begin() + left_count, align);
    const auto right_result =
        process_some(images.rbegin(), images.rbegin() + right_count, align);

    if (left_result.succeeded() && right_result.succeeded()) {
      return align_and_stack(left_result, right_result, align);
    }
    std::cerr << "Can't align and stack.  At least one partial result is empty."
              << std::endl;

    return {};
  }

  [[nodiscard]] StackedImage process_some(const auto begin, const auto end,
                                          bool align) const {
    auto fut_iter = begin;
    const auto info = fut_iter->get();
    std::cout << info->path() << std::endl;

    // At every step, (align and) stack the pile of images already processed,
    // onto the next image.  This shifts the whole pile of processed images, a
    // little at a time, to align it with the next image in the sequence.

    StackedImage result(info->image());

    for (++fut_iter; fut_iter != end; ++fut_iter) {
      const auto next_info(fut_iter->get());
      StackedImage next_image(next_info->image());
      std::cout << next_info->path() << std::endl;

      result = align_and_stack(result, next_image, align);
    }
    return result;
  }

  [[nodiscard]] StackedImage align_and_stack(const StackedImage &unaligned,
                                             const StackedImage &target,
                                             bool align) const {

    if (unaligned.empty() || target.empty()) {
      report_empty();
      return {};
    }
    if (!unaligned.same_extents(target)) {
      report_size_mismatch(target.m_image, unaligned.m_image, "image pair");
      return unaligned;
    }

    if (align) {
      ImageAligner aligner;
      cv::Mat aligned_image; // Will hold internal_image, aligned to
                             // internal_target.
      aligner.align(target.m_image, unaligned.m_image, aligned_image);

      StackedImage aligned(aligned_image, unaligned.m_num_used);
      return stack_pair(aligned, target);
    }

    return stack_pair(unaligned, target);
  }

  [[nodiscard]] StackedImage stack_pair(const auto &top_image,
                                        const auto &bottom_image) const {
    return {top_image.m_image + bottom_image.m_image,
            top_image.m_num_used + bottom_image.m_num_used};
  }
};
} // namespace

ImageStacker::Ptr ImageStacker::create() { return std::make_unique<Impl>(); }

} // namespace StackExposures
