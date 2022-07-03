#include <filesystem>
#include <iostream>
#include <sstream>

#include <deque>
#include <future>
#include <semaphore>

#include <opencv2/imgcodecs.hpp>

#include "cmd_options.hpp"
#include "image_aligner.hpp"
#include "image_loader.hpp"
#include "image_stacker.hpp"

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace StackExposures;

  CmdLine::CmdOptions opts(CmdLine::arg_vector(argc, (const char **)argv));

  using ImageInfoFuture = shared_future<ImageInfo::Ptr>;

  constexpr size_t max_concurrent_loads = 4;
  counting_semaphore gate(max_concurrent_loads);
  deque<ImageInfoFuture> load_tasks;
  for (const auto image_path : opts.input_images()) {
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
        auto aligned = opts.align() ? aligner.align(img_info) : img_info;
        stacker.push(aligned->image());
      }
    }
  }

  cout << "Saving result to " << opts.outpath() << endl;
  cv::imwrite(opts.outpath().c_str(), stacker.result());
  return 0;
}