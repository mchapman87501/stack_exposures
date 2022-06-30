#include <filesystem>
#include <iostream>
#include <sstream>

#include <deque>
#include <future>
#include <semaphore>

#include <opencv2/imgcodecs.hpp>

#include "image_aligner.hpp"
#include "image_loader.hpp"
#include "image_stacker.hpp"

namespace {
using namespace std;

filesystem::path default_outpath("stacked.tiff");

struct Options {
  string m_invoked_as;
  bool m_align;
  filesystem::path m_outpath;
  vector<filesystem::path> m_input_images;

  Options(int argc, char *argv[]) : m_align(true), m_outpath(default_outpath) {

    m_invoked_as = string(argv[0]);
    int i = 1;
    while (i < argc) {
      string curr(argv[i]);
      if (curr == "-o") {
        i++;
        m_outpath = filesystem::path(argv[i]);
        // TODO handle "--output", "--output="
      } else if (curr == "--no-align") {
        m_align = false;
      } else if ((curr == "-h") || (curr == "--help")) {
        show_help();
      } else if (curr.front() == '-') {
        ostringstream outs;
        outs << "Unknown option '" << curr << "'.";
        show_help(outs.str());
      } else {
        m_input_images.push_back(filesystem::path(curr));
      }
      i++;
    }

    if (m_input_images.size() < 1) {
      show_help(cerr, 1);
    }
  }

private:
  string padded_right(const string &s, size_t width) {
    if (width <= s.size()) {
      return s;
    }
    size_t pad_count(width - s.size());
    string padding(pad_count, ' ');
    return s + padding;
  }

  string padded_arg_help(const string arg, const string &help_msg,
                         size_t pad_width = 20) {
    return padded_right(arg, pad_width) + help_msg;
  }

  void show_help(ostream &outs = cout, int exit_code = 0) {
    // O for std::format
    ostringstream op_outs;
    op_outs << "Save result to <path>; default " << default_outpath << ".";
    const string output_path_help(op_outs.str());

    outs << "Usage: " << m_invoked_as
         << " [-o path] [--no-align] [-h|--help] image_filaname "
            "[image_filename...]"
         << endl
         << "Options:" << endl
         << padded_arg_help("  -o <path>", output_path_help) << endl
         << padded_arg_help("  --no-align",
                            "Do not align images before stacking.")
         << endl
         << padded_arg_help("  -h, --help", "Show this help message and exit.")
         << endl
         << "Arguments:" << endl
         << padded_arg_help("  <image_filename>", "path of an image to stack")
         << endl;
    std::exit(exit_code);
  }

  void show_help(const string &error_msg) {
    cerr << "Error: " << error_msg << endl;
    show_help(cerr, 1);
  }
};

} // namespace

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace StackExposures;

  Options opts(argc, argv);

  ImageAligner aligner;
  ImageStacker stacker;

  using ImageInfoFuture = shared_future<ImageInfo::Ptr>;

  counting_semaphore gate(4);
  deque<ImageInfoFuture> load_tasks;
  // TODO use a thread pool instead of launching all background loads at once.
  for (const auto image_path : opts.m_input_images) {
    ImageLoader loader;
    // Support concurrent loading/alignment/stacking, while
    // respecting stacking order.
    auto load_async = [&gate, image_path]() {
      ImageLoader loader;
      gate.acquire();
      auto result = loader.load_image(image_path);
      gate.release();
      return result;
    };
    load_tasks.push_back(async(launch::async, load_async));
  }

  bool is_first = true;
  while (!load_tasks.empty()) {
    auto fut = load_tasks.front();
    load_tasks.pop_front();
    auto img_info = fut.get();

    cout << img_info->m_path << endl << flush;
    // TODO get the icc profile from the first image.
    if (is_first) {
    }

    auto aligned = opts.m_align ? aligner.align(img_info) : img_info;
    stacker.push(aligned->image());

    is_first = false;
  }

  // What should be the name of the output image?
  cout << "Saving result to " << opts.m_outpath << endl;
  cv::imwrite(opts.m_outpath.c_str(), stacker.result());
  return 0;
}