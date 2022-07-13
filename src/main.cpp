#include <iostream>

#include <deque>
#include <future>
#include <semaphore>

#include <opencv2/imgcodecs.hpp>

#include "arg_parse.hpp"
#include "image_aligner.hpp"
#include "image_loader.hpp"
#include "image_stacker.hpp"

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace StackExposures;

auto parser = ArgParse::ArgumentParser::create("Stack images into a single image.");
auto no_align = ArgParse::flag(parser, "--no-align", "--no-align", "Skip aligning images before stacking.");

const std::string default_out_pathname("stacked.tiff");
const std::filesystem::path default_out_path(default_out_pathname);
const auto outpath_help = "Where to save the result; default '" + default_out_pathname + "'.";
auto output_path = ArgParse::option<std::filesystem::path>(parser, "-o", "--output-path", outpath_help);

auto input_images = ArgParse::argument<std::filesystem::path>(parser, "image", ArgParse::Nargs::one_or_more, "Stack these images.");

parser->parse_args(argc, argv);
if (parser->should_exit()) {
    return parser->exit_code();
}

  using ImageInfoFuture = shared_future<ImageInfo::Ptr>;

  constexpr size_t max_concurrent_loads = 4;
  counting_semaphore gate(max_concurrent_loads);
  deque<ImageInfoFuture> load_tasks;
  for (const auto image_path : input_images->values()) {
    ImageLoader loader;
    auto load_async = [&gate, image_path]() {
      ImageLoader loader;
      gate.acquire();
      auto result = loader.load_image(image_path);
      gate.release();
      return result;
    };
    load_tasks.push_back(async(launch::async, load_async));
  }

  ImageAligner aligner;
  ImageStacker stacker;

const bool align = !no_align->is_set();
  bool is_first = true;
  ImageInfo::Ptr ref_img(nullptr);

  while (!load_tasks.empty()) {
    auto fut = load_tasks.front();
    load_tasks.pop_front();
    auto img_info = fut.get();
    cout << img_info->path() << endl << flush;

    if (is_first) {
      // TODO get the icc profile from the first image.
      ref_img = img_info;
      stacker.push(img_info->image());
      is_first = false;
    } else {

      if (!ref_img->same_extents(img_info)) {
        cerr << "Cannot process " << img_info->path()
             << ": image width x height (" << img_info->cols() << " x "
             << img_info->rows() << ") do not match first image ("
             << ref_img->cols() << " x " << ref_img->rows() << ")" << endl;
      } else {
        auto aligned = align ? aligner.align(img_info) : img_info;
        stacker.push(aligned->image());
      }
    }
  }

// TODO arg_parse's Option needs to support default values.
const std::string selected_outpath = (output_path->value().string() != "") ? output_path->value().string() : default_out_pathname;
  cout << "Saving result to " << selected_outpath << endl;
  cv::imwrite(selected_outpath, stacker.result());
  return 0;
}