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

IImageStacker::Ptr image_stacker(std::string_view method_id, double gamma) {
  if (method_id == "m") {
    return ImageStacker::mean(gamma);
  }
  if (method_id == "s") {
    return ImageStacker::stretch(gamma);
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

auto stacked_result(const IImageStacker::Ptr &stacker,
                    std::string_view filename) {
  auto suffix = StrUtil::lowercase(filename_suffix(filename));
  if ((suffix == ".tiff") || (suffix == ".tif")) {
    return stacker->result16();
  }
  if (suffix == ".png") {
    return stacker->result16();
  }
  if (suffix == ".jpg") {
    return stacker->result8();
  }
  // TODO HDR
  // Play it safe.
  return stacker->result8();
}
} // namespace

int main(int argc, char *argv[]) {
  CmdOption opt(argc, argv);
  if (opt.should_exit()) {
    return opt.exit_code();
  }

  ImageAligner aligner;
  IImageStacker::Ptr stacker = image_stacker(opt.method(), opt.gamma());

  ImageInfo::SharedPtr ref_img(nullptr);

  AsyncImageLoader loader(opt.images());
  for (auto fut : loader.futures()) {
    ImageInfo::SharedPtr img_info = fut.get();
    std::cout << img_info->path() << std::endl << std::flush;

    if (!ref_img) {
      ref_img = img_info;
    }

    if (ref_img->same_extents(img_info)) {
      auto aligned = opt.align() ? aligner.align(img_info) : img_info;
      if (aligned != nullptr) {
        stacker->add(aligned->image());
      }
    } else {
      report_size_mismatch(ref_img, img_info);
    }
  }

  if (!opt.dark_image().empty()) {
    ImageLoader loader;
    auto dark_image = loader.load_image(opt.dark_image());
    stacker->subtract(dark_image->image());
  }

  const auto output_pathname(opt.output_pathname());
  const auto final_image = stacked_result(stacker, output_pathname.string());
  if (final_image.empty()) {
    std::cerr << "Final stack image is empty." << std::endl;
    return 2;
  }
  cv::imwrite(output_pathname, final_image);
  return 0;
}
