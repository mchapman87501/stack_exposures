#include <iostream>

#include <cctype>
#include <deque>
#include <future>
#include <semaphore>

#include <opencv2/imgcodecs.hpp>

#include "arg_parse.hpp"
#include "image_aligner.hpp"
#include "image_loader.hpp"
#include "image_stacker.hpp"
#include "str_util.hpp"

using namespace StackExposures;

namespace {
using ImageInfoFuture = std::shared_future<ImageInfo::SharedPtr>;

static const std::string default_out_pathname("stacked.tiff");
static constexpr double default_gamma(2.4);

class CmdOption {
  ArgParse::ArgumentParser::Ptr m_parser;
  ArgParse::Flag::Ptr m_no_align;
  ArgParse::Option<double>::Ptr m_gamma;
  ArgParse::Choice::Ptr m_method;
  ArgParse::Option<std::filesystem::path>::Ptr m_output_path;
  ArgParse::Option<std::filesystem::path>::Ptr m_dark_image;
  ArgParse::Argument<std::filesystem::path>::Ptr m_input_images;

public:
  CmdOption(int argc, char **argv) {
    m_parser =
        ArgParse::ArgumentParser::create("Stack images into a single image.");

    m_no_align = ArgParse::flag(m_parser, "--no-align", "--no-align",
                                "Skip aligning images before stacking.");

    m_gamma = ArgParse::option<double>(
        m_parser, "-g", "--gamma",
        "gamma to apply to final stacked image.  Default " +
            std::to_string(default_gamma),
        default_gamma);

    m_method = ArgParse::choice(m_parser, "-s", "--stacking-method",
                                "stacking method: 'm' == mean, 's' == scaled.",
                                {"m", "s"});

    m_dark_image = ArgParse::option<std::filesystem::path>(
        m_parser, "-d", "--dark-image",
        "Dark image to be subtracted from the exposure.");

    const std::filesystem::path default_out_path(default_out_pathname);
    const auto outpath_help =
        "Where to save the result; default '" + default_out_pathname + "'.";
    m_output_path = ArgParse::option<std::filesystem::path>(
        m_parser, "-o", "--output-path", outpath_help);

    m_input_images = ArgParse::argument<std::filesystem::path>(
        m_parser, "image", ArgParse::Nargs::one_or_more, "Stack these images.");

    m_parser->parse_args(argc, argv);
  }

  [[nodiscard]] bool should_exit() const { return m_parser->should_exit(); }

  [[nodiscard]] int exit_code() const { return m_parser->exit_code(); }

  [[nodiscard]] auto dark_image() const { return m_dark_image->value(); }

  [[nodiscard]] auto images() const { return m_input_images->values(); }

  [[nodiscard]] bool align() const { return !m_no_align->is_set(); }

  [[nodiscard]] std::string method() const { return m_method->value(); }

  [[nodiscard]] double gamma() const { return m_gamma->value(); }

  [[nodiscard]] std::filesystem::path output_pathname() const {
    return m_output_path->value();
  }
};

struct AsyncImageLoader {
  AsyncImageLoader(std::vector<std::filesystem::path> image_paths)
      : m_gate(max_concurrent_loads) {
    for (const auto image_path : image_paths) {
      auto load_async = [this, image_path]() {
        m_gate.acquire();
        ImageLoader loader;
        auto result = loader.load_image(image_path);
        m_gate.release();
        return result;
      };

      m_futures.emplace_back(std::async(std::launch::async, load_async));
    }
  }

  auto futures() const { return m_futures; }

private:
  constexpr static size_t max_concurrent_loads = 4;
  std::counting_semaphore<max_concurrent_loads> m_gate;
  std::deque<ImageInfoFuture> m_futures;
};

void report_size_mismatch(ImageInfo::SharedPtr ref_img,
                          ImageInfo::SharedPtr img_info) {
  std::cerr << "Cannot process " << img_info->path()
            << ": image width x height (" << img_info->cols() << " x "
            << img_info->rows() << ") do not match first image ("
            << ref_img->cols() << " x " << ref_img->rows() << ")" << std::endl;
}

using StackerFactory = std::function<IImageStacker::Ptr()>;

struct MainStacker {
  MainStacker(std::deque<ImageInfoFuture> &images, StackerFactory new_stacker,
              bool align)
      : m_images(images), m_new_stacker(std::move(new_stacker)),
        m_align(align) {}

  void add_dark_image(ImageInfo::SharedPtr dark_image) {
    m_dark_image = dark_image;
  }

  [[nodiscard]] cv::Mat result8() { return make_final_stack()->result8(); }

  [[nodiscard]] cv::Mat result16() { return make_final_stack()->result16(); }

private:
  const std::deque<ImageInfoFuture> &m_images;
  const StackerFactory m_new_stacker;
  const bool m_align;

  ImageInfo::SharedPtr m_dark_image;

  [[nodiscard]] IImageStacker::Ptr make_final_stack() const {
    auto result = m_new_stacker();
    bool succeeded{false};
    ImageInfo::SharedPtr stacked_image{};
    process_all(succeeded, stacked_image);
    if (succeeded) {
      result->add(stacked_image->image() / m_images.size());
      if ((m_dark_image != nullptr) && !m_dark_image->empty()) {
        result->subtract(m_dark_image->image());
      }
    }
    return result;
  }

  void process_all(bool &succeeded, ImageInfo::SharedPtr &stacked_image) const {
    succeeded = false;
    const auto count = m_images.size();
    if (count < 3) {
      if (count < 1) {
        std::cerr << "Can't align and stack -- need at least two images."
                  << std::endl;
      } else if (count == 1) {
        stacked_image = m_images.front().get();
        succeeded = true;
      } else {
        stacked_image = process_one(m_images.front().get(), 1,
                                    m_images.back().get(), m_align);
        succeeded = true;
      }
      return;
    }

    const auto i_center = count / 2;

    // Alas, clang 14 doesn't support std::views.
    // TODO write tests to verify that all images are processed.
    const auto left_result =
        process_some(m_images.begin(), m_images.begin() + i_center, m_align);
    const auto num_to_process = i_center + (((2 * i_center) < count) ? 1 : 0);
    const auto right_result = process_some(
        m_images.rbegin(), m_images.rbegin() + num_to_process, m_align);

    if (!(left_result->empty() || right_result->empty())) {
      stacked_image = process_one(left_result, 1, right_result, m_align);
      succeeded = true;
    } else {
      std::cerr
          << "Can't align and stack.  At least one partial result is empty."
          << std::endl;
    }
  }

  [[nodiscard]] ImageInfo::SharedPtr
  process_some(const auto begin, const auto end, bool align) const {
    size_t count = 1;
    auto fut_iter = begin;
    auto result = fut_iter->get();
    std::cout << result->path() << std::endl;
    for (++fut_iter; fut_iter != end; ++fut_iter) {
      auto image(fut_iter->get());
      std::cout << image->path() << std::endl;
      result = process_one(result, count, image, align);
      count += 1;
    }
    return result;
  }

  [[nodiscard]] ImageInfo::SharedPtr
  process_one(const ImageInfo::SharedPtr image, const size_t image_stack_count,
              const ImageInfo::SharedPtr ref_image, bool align) const {

    if (!ref_image->same_extents(image)) {
      report_size_mismatch(ref_image, image);
      return ref_image;
    }

    const auto internal_ref_image = to_stacking_format(ref_image);
    const auto internal_image = to_stacking_format(image);
    if (align) {
      ImageAligner aligner;
      const auto ref_aligned = aligner.align(internal_ref_image);
      const auto image_aligned = aligner.align(internal_image);
      if ((ref_aligned != nullptr) && (image_aligned != nullptr)) {
        return stack_pair(image_aligned, ref_aligned);
      }
    } else {
      if ((internal_image != nullptr) && (internal_ref_image != nullptr)) {
        return stack_pair(internal_image, internal_ref_image);
      }
    }

    return ref_image;
  }

  [[nodiscard]] ImageInfo::SharedPtr
  to_stacking_format(const ImageInfo::SharedPtr image) const {
    cv::Mat internal_image;
    image->image().convertTo(internal_image, CV_32FC3);
    return ImageInfo::with_image(*image, internal_image);
  }

  [[nodiscard]] ImageInfo::SharedPtr stack_pair(const auto image,
                                                const auto bottom_image) const {
    auto stacker = m_new_stacker();
    stacker->add(bottom_image->image());
    stacker->add(image->image());
    return ImageInfo::with_image(*image, stacker->partial_sum());
  }
};

StackerFactory image_stacker(std::string_view method_id, double gamma) {
  if (method_id == "m") {
    return [gamma]() { return ImageStacker::mean(gamma); };
  }
  if (method_id == "s") {
    return [gamma]() { return ImageStacker::stretch(gamma); };
  }
  throw std::runtime_error("Unsupported STACKING_METHOD");
}

std::string filename_suffix(std::string_view filename) {
  auto index = filename.find_last_of('.');
  if (index != std::string::npos) {
    return std::string(filename.substr(index));
  }
  return "";
}

auto stacked_result(MainStacker &stacker, std::string_view filename) {
  auto suffix = StrUtil::lowercase(filename_suffix(filename));
  if ((suffix == ".tiff") || (suffix == ".tif")) {
    return stacker.result16();
  }
  if (suffix == ".png") {
    return stacker.result16();
  }
  if (suffix == ".jpg") {
    return stacker.result8();
  }
  // TODO HDR
  // Play it safe.
  return stacker.result8();
}
} // namespace

int main(int argc, char *argv[]) {
  CmdOption opt(argc, argv);
  if (opt.should_exit()) {
    return opt.exit_code();
  }

  AsyncImageLoader loader(opt.images());
  auto futures = loader.futures();
  MainStacker main_stacker(futures, image_stacker(opt.method(), opt.gamma()),
                           opt.align());

  if (!opt.dark_image().empty()) {
    ImageLoader loader;
    auto dark_image = loader.load_image(opt.dark_image());
    main_stacker.add_dark_image(dark_image);
  }

  const auto output_pathname(opt.output_pathname());
  const auto final_image =
      stacked_result(main_stacker, output_pathname.string());
  if (final_image.empty()) {
    std::cerr << "Final stack image is empty." << std::endl;
    return 2;
  }
  cv::imwrite(output_pathname, final_image);
  return 0;
}
