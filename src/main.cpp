#include <iostream>

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

auto load_images(std::vector<std::filesystem::path> &&image_paths) {
  // Widen the image-loading bottleneck, without having too many
  // images in memory at once.
  constexpr size_t max_concurrent_loads = 4;
  counting_semaphore gate(max_concurrent_loads);

  deque<ImageInfoFuture> result;
  for (const auto image_path : image_paths) {
    ImageLoader loader;
    auto load_async = [&gate, image_path]() {
      ImageLoader loader;
      gate.acquire();
      auto result = loader.load_image(image_path);
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
} // namespace

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace StackExposures;

  auto parser =
      ArgParse::ArgumentParser::create("Stack images into a single image.");
  auto no_align = ArgParse::flag(parser, "--no-align", "--no-align",
                                 "Skip aligning images before stacking.");

  const std::string default_out_pathname("stacked.tiff");
  const std::filesystem::path default_out_path(default_out_pathname);
  const auto outpath_help =
      "Where to save the result; default '" + default_out_pathname + "'.";
  auto output_path = ArgParse::option<std::filesystem::path>(
      parser, "-o", "--output-path", outpath_help);

  auto input_images = ArgParse::argument<std::filesystem::path>(
      parser, "image", ArgParse::Nargs::one_or_more, "Stack these images.");

  parser->parse_args(argc, argv);
  if (parser->should_exit()) {
    return parser->exit_code();
  }

  auto image_futures = load_images(input_images->values());

  ImageAligner aligner;
  ImageStacker stacker;

  const bool align = !no_align->is_set();
  ImageInfo::Ptr ref_img(nullptr);

  while (!image_futures.empty()) {
    auto fut = image_futures.front();
    image_futures.pop_front();
    auto img_info = fut.get();
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

  // TODO arg_parse's Option needs to support default values.
  const std::string selected_outpath = (output_path->value().empty())
                                           ? default_out_pathname
                                           : output_path->value().string();
  cout << "Saving result to " << selected_outpath << endl;
  cv::imwrite(selected_outpath, stacker.result());
  return 0;
}