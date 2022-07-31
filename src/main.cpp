#include <iostream>

#include <cctype>
#include <future>
#include <semaphore>

#include <opencv2/imgcodecs.hpp>

#include "arg_parse.hpp"
#include "image_info.hpp"
#include "image_loader.hpp"
#include "image_stacker.hpp"
#include "str_util.hpp"

using namespace StackExposures;

namespace {

const std::string default_out_pathname("stacked.tiff");
const std::vector<std::string> supported_extensions{".tif", ".tiff", ".png",
                                                    ".jpg", ".jpeg"};

auto lowercase_extension(std::string_view filename) {
  std::filesystem::path pathname(filename);

  return StrUtil::lowercase(pathname.extension().string());
}

bool supported_format(std::string_view image_filename) {
  const auto ext = lowercase_extension(image_filename);
  return (std::find(supported_extensions.begin(), supported_extensions.end(),
                    ext) != supported_extensions.end());
}

class CmdOption {
  ArgParse::ArgumentParser::Ptr m_parser;
  ArgParse::Flag::Ptr m_no_align;
  ArgParse::Option<std::filesystem::path>::Ptr m_output_path;
  ArgParse::Option<std::filesystem::path>::Ptr m_dark_image;
  ArgParse::Argument<std::filesystem::path>::Ptr m_input_images;

public:
  CmdOption(int argc, char **argv) {
    m_parser =
        ArgParse::ArgumentParser::create("Stack images into a single image.");

    m_no_align = ArgParse::flag(m_parser, "--no-align", "--no-align",
                                "Skip aligning images before stacking.");

    m_dark_image = ArgParse::option<std::filesystem::path>(
        m_parser, "-d", "--dark-image",
        "Dark image to be subtracted from the exposure.");

    const std::filesystem::path default_out_path(default_out_pathname);
    const auto outpath_help =
        "Where to save the result; default '" + default_out_pathname + "'.";
    m_output_path = ArgParse::option<std::filesystem::path>(
        m_parser, "-o", "--output-path", outpath_help, default_out_path);

    m_input_images = ArgParse::argument<std::filesystem::path>(
        m_parser, "image", ArgParse::Nargs::one_or_more, "Stack these images.");

    m_parser->parse_args(argc, argv);

    const auto path_str(m_output_path->value().string());
    if (!supported_format(path_str)) {
      std::ostringstream outs;
      outs << "Output format '" << lowercase_extension(path_str)
           << "' is not supported." << std::endl
           << "Please use one of these extensions when specifying output-path:";
      for (const auto &ext : supported_extensions) {
        outs << " '" << ext << "'";
      }
      m_parser->show_error(outs.str(), 1);
    }
  }

  [[nodiscard]] bool should_exit() const { return m_parser->should_exit(); }

  [[nodiscard]] int exit_code() const { return m_parser->exit_code(); }

  [[nodiscard]] auto dark_image() const { return m_dark_image->value(); }

  [[nodiscard]] auto images() const { return m_input_images->values(); }

  [[nodiscard]] bool align() const { return !m_no_align->is_set(); }

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
  ImageInfoFutureContainer m_futures;
};

auto formatted_for_output(const cv::Mat &stacking_image,
                          std::string_view filename) {
  // stacking_image should be in MainStacker's format, which will use
  // values in 0...255.

  cv::Mat scaled(stacking_image);
  auto output_type{CV_8UC3};

  std::filesystem::path pathname(filename);

  auto suffix = StrUtil::lowercase(pathname.extension().string());
  if ((suffix == ".tiff") || (suffix == ".tif") || (suffix == ".png")) {
    scaled = stacking_image * 0xFF;
    output_type = CV_16UC3;
  }

  cv::Mat result;
  scaled.convertTo(result, output_type);
  return result;
}
} // namespace

int main(int argc, char *argv[]) {
  CmdOption opt(argc, argv);
  if (opt.should_exit()) {
    return opt.exit_code();
  }

  AsyncImageLoader loader(opt.images());
  auto futures = loader.futures();

  ImageInfo::SharedPtr dark_image{};
  if (!opt.dark_image().empty()) {
    ImageLoader loader;
    dark_image = loader.load_image(opt.dark_image());
  }

  auto stacker = ImageStacker::create();
  const auto processed_image =
      stacker->stacked_result(futures, dark_image, opt.align());

  const auto output_pathname(opt.output_pathname().string());
  const auto final_image =
      formatted_for_output(processed_image, output_pathname);
  if (final_image.empty()) {
    std::cerr << "Final stack image is empty." << std::endl;
    return 2;
  }
  cv::imwrite(output_pathname, final_image);
  return 0;
}
