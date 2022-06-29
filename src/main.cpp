#include <filesystem>
#include <iostream>
#include <sstream>

#include <opencv2/imgcodecs.hpp>

#include "image_aligner.hpp"
#include "image_loader.hpp"
#include "image_stacker.hpp"

namespace {
    using namespace std;

    filesystem::path default_outpath("stacked.png");

    struct Options {
        string m_invoked_as;
        filesystem::path m_outpath;
        vector<filesystem::path> m_input_images;

        Options(int argc, char *argv[]):
            m_outpath(default_outpath) {

            m_invoked_as = string(argv[0]);
            int i = 1;
            while (i < argc) {
                string curr(argv[i]);
                if (curr == "-o") {
                    i++;
                    m_outpath = filesystem::path(argv[i]);
                // TODO handle "--output", "--output="
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
        string padded_right(const string& s, size_t width) {
            if (width <= s.size()) {
                return s;
            }
            size_t pad_count(width - s.size());
            string padding(pad_count, ' ');
            return s + padding;
        }

        string padded_arg_help(const string arg, const string& help_msg, size_t pad_width = 20) {
            return padded_right(arg, pad_width) + help_msg;
        }

        void show_help(ostream& outs = cout, int exit_code = 0) {
            // O for std::format
            ostringstream op_outs;
            op_outs << "Save result to <path>; default " << default_outpath;
            const string output_path_help(op_outs.str());

            outs << "Usage: " << m_invoked_as << " [-o path] [-h|--help] image_filaname [image_filename...]" << endl
            << "Options:" << endl
            << padded_arg_help("  -o <path>", output_path_help) << endl
            << padded_arg_help("  -h, --help", "Show this help message and exit.") << endl
            << "Arguments:" << endl
            << padded_arg_help("  <image_filename>", "path of an image to stack") << endl;
            std::exit(exit_code);
        }

        void show_help(const string& error_msg) {
            cerr << "Error: " << error_msg << endl;
            show_help(cerr, 1);
        }
    };

}

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace StackExposures;

  Options opts(argc, argv);

  ImageLoader loader;
  ImageAligner aligner;
  ImageStacker stacker;

  bool is_first = true;
  for (const auto image_path: opts.m_input_images) {
    cout << "Processing " << image_path << endl;

    // TODO get the icc profile from the first image.
    if (is_first) {
    }

    // TODO support concurrent loading/alignment/stacking.
    auto img_info = loader.load_image(image_path);
    auto aligned = aligner.align(img_info);
    stacker.push(aligned->image());

    is_first = false;
  }

  // What should be the name of the output image?
  cout << "Saving result to " << opts.m_outpath << endl;
  cv::imwrite(opts.m_outpath.c_str(), stacker.result());
  return 0;
}