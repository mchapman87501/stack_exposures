#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace CmdLine {
const std::filesystem::path default_outpath("stacked.tiff");

using ArgVec = std::vector<const std::string>;

ArgVec arg_vector(int argc, const char *argv[]);

struct CmdOptions {
  CmdOptions(const ArgVec &args);

  bool align() const { return m_align; }
  const auto outpath() const { return m_outpath; }
  const auto input_images() const { return m_input_images; }

private:
  std::string m_invoked_as;
  bool m_align;
  std::filesystem::path m_outpath;
  std::vector<std::filesystem::path> m_input_images;

  void show_help(std::ostream &outs = std::cout, int exit_code = 0);
  void show_help(std::string_view error_msg, int exit_code = 1);
};

} // namespace CmdLine