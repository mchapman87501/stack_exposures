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

namespace {
using namespace std;
using namespace StackExposures;

using ImageInfoFuture = shared_future<ImageInfo::Ptr>;

namespace {
static const std::string default_out_pathname("stacked.tiff");
}
class CmdOption {
  ArgParse::ArgumentParser::Ptr m_parser;
  ArgParse::Flag::Ptr m_no_align;
  ArgParse::Option<std::filesystem::path>::Ptr m_output_path;
  ArgParse::Argument<std::filesystem::path>::Ptr m_input_images;

public:
  CmdOption(int argc, char **argv) {
    m_parser =
        ArgParse::ArgumentParser::create("Stack images into a single image.");
    m_no_align = ArgParse::flag(m_parser, "--no-align", "--no-align",
                                "Skip aligning images before stacking.");

    const std::filesystem::path default_out_path(default_out_pathname);
    const auto outpath_help =
        "Where to save the result; default '" + default_out_pathname + "'.";
    m_output_path = ArgParse::option<std::filesystem::path>(
        m_parser, "-o", "--output-path", outpath_help);

    m_input_images = ArgParse::argument<std::filesystem::path>(
        m_parser, "image", ArgParse::Nargs::one_or_more, "Stack these images.");

    m_parser->parse_args(argc, argv);
  }

  bool should_exit() const { return m_parser->should_exit(); }

  int exit_code() const { return m_parser->exit_code(); }

  auto images() const { return m_input_images->values(); }

  bool align() const { return !m_no_align->is_set(); }

  std::string const output_pathname() {
    return (m_output_path->value().empty()) ? default_out_pathname
                                            : m_output_path->value().string();
  }
};

// Widen the image-loading bottleneck, without having too many
// images in memory at once.
constexpr size_t max_concurrent_loads = 4;
counting_semaphore gate(max_concurrent_loads);

auto load_images(vector<filesystem::path> image_paths) {
  deque<ImageInfoFuture> result;
  for (const auto image_path : image_paths) {
    auto load_async = [image_path]() {
      gate.acquire();
      auto my_loader = ImageLoader::create();
      auto result = my_loader->load_image(image_path);
      gate.release();
      return result;
    };

    result.push_back(async(launch::async, load_async));
  }
  return result;
}

void report_size_mismatch(ImageInfo::Ptr ref_img, ImageInfo::Ptr img_info) {
  cerr << "Cannot process " << img_info->path() << ": image width x height ("
       << img_info->cols() << " x " << img_info->rows()
       << ") do not match first image (" << ref_img->cols() << " x "
       << ref_img->rows() << ")" << endl;
}

std::string filename_suffix(std::string_view filename) {
  auto index = filename.find_last_of('.');
  if (index != std::string::npos) {
    return std::string(filename.substr(index));
  }
  return "";
}

auto lowercase(std::string_view sval) {
  std::string result;
  std::transform(sval.begin(), sval.end(), result.end(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

auto stacked_result(const ImageStacker &stacker, std::string_view filename) {
  auto suffix = lowercase(filename_suffix(filename));
  if (suffix == ".tiff") {
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
  using namespace std;
  using namespace StackExposures;

  CmdOption opt(argc, argv);
  if (opt.should_exit()) {
    return opt.exit_code();
  }

  auto image_futures = load_images(opt.images());

  ImageAligner aligner;
  ImageStacker stacker;

  const bool align = opt.align();
  ImageInfo::Ptr ref_img(nullptr);

  while (!image_futures.empty()) {
    auto fut = image_futures.front();
    image_futures.pop_front();
    ImageInfo::Ptr img_info = fut.get();
    cout << img_info->path() << endl << flush;

    if (!ref_img) {
      // TODO get the icc profile from the first image.
      ref_img = img_info;
      stacker.push(img_info->image());
    } else {

      if (!ref_img->same_extents(img_info)) {
        report_size_mismatch(ref_img, img_info);
      } else {
        auto aligned = align ? aligner.align(img_info) : img_info;
        stacker.push(aligned->image());
      }
    }
  }

  const std::string output_pathname(opt.output_pathname());
  cout << "Saving result to " << output_pathname << endl;
  // TODO Figure out how to support multiple bit depths/formats from cmdline.
  // May need to introduce a Choice subclass of arg_parse::IOption.
  cv::imwrite(output_pathname, stacked_result(stacker, output_pathname));
  return 0;
}