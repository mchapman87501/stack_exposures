#include "cmd_options.hpp"
#include <sstream>

namespace {
using namespace std;

string padded_right(string_view s, size_t width) {
  if (width <= s.size()) {
    return string(s);
  }
  size_t pad_count(width - s.size());
  string padding(pad_count, ' ');
  return string(s) + padding;
}

string padded_arg_help(string_view arg, string_view help_msg,
                       size_t pad_width = 24) {
  return padded_right(arg, pad_width) + " " + help_msg.data();
}

} // namespace

namespace CmdLine {
using namespace std;

ArgVec arg_vector(int argc, const char *argv[]) {
  ArgVec result;
  for (int i = 0; i < argc; ++i) {
    result.push_back(argv[i]);
  }
  return result;
}

CmdOptions::CmdOptions(const ArgVec &args)
    : m_align(true), m_outpath(default_outpath) {
  const string output_eq("--output=");

  if (args.empty()) {
    show_error("Internal Error: empty args vector", 2);
    return;
  }
  m_invoked_as = args[0];

  size_t i = 1;
  const size_t i_max(args.size());
  while (i < args.size()) {
    const string &curr(args[i]);
    if ((curr == "-o") || (curr == "--output")) {
      i++;
      if (i >= i_max) {
        show_error("No output path was provided: '" + curr + "'", 1);
        return;
      }
      m_outpath = filesystem::path(args[i]);
    } else if (curr.starts_with(output_eq)) {
      const string outpath = curr.substr(output_eq.size());
      if (outpath.empty()) {
        show_error("No output path was provided: '" + curr + "'", 1);
        return;
      }
      m_outpath = filesystem::path(outpath);
    } else if (curr == "--no-align") {
      m_align = false;
    } else if ((curr == "-h") || (curr == "--help")) {
      show_help(cout, 0);
      return;
    } else if (curr.front() == '-') {
      ostringstream outs;
      outs << "Unknown option '" << curr << "'.";
      show_error(outs.str(), 1);
      return;
    } else {
      m_input_images.push_back(filesystem::path(curr));
    }
    i++;
  }

  if (m_input_images.size() < 1) {
    show_error("Please provide at least one input image.", 1);
    return;
  }
}

void CmdOptions::show_help(ostream &outs, int exit_code) {
  // O for std::format
  ostringstream op_outs;
  op_outs << "Save result to <path>; default " << default_outpath << ".";
  const string output_path_help(op_outs.str());

  outs << "Usage: " << m_invoked_as
       << " [-o|--output path] [--no-align] [-h|--help] image_filename "
          "[image_filename...]"
       << endl
       << "CmdOptions:" << endl
       << padded_arg_help("  -o|--output <path>", output_path_help) << endl
       << padded_arg_help("  --no-align",
                          "Do not align images before stacking.")
       << endl
       << padded_arg_help("  -h, --help", "Show this help message and exit.")
       << endl
       << "Arguments:" << endl
       << padded_arg_help("  <image_filename>", "path of an image to stack")
       << endl;
  m_exit_code = exit_code;
}

void CmdOptions::show_error(string_view error_msg, int exit_code) {
  cerr << "Error: " << error_msg << endl;
  show_help(cerr, exit_code);
}

} // namespace CmdLine
