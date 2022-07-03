#include "cmd_options.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <cstdio>
#include <iostream>
#include <sstream>

namespace {
struct SystemExit : public std::runtime_error {
  const int m_exit_code;
  SystemExit(int exit_code)
      : m_exit_code(exit_code), std::runtime_error(std::to_string(exit_code)) {}
};

// See https://gist.github.com/AndreLouisCaron/1841061
class Redirect {
  std::ostream &m_stream;
  std::streambuf *const m_buff;

public:
  Redirect(std::ostream &lhs, std::ostream &rhs = std::cout)
      : m_stream(rhs), m_buff(m_stream.rdbuf()) {
    m_stream.rdbuf(lhs.rdbuf());
  }

  ~Redirect() { m_stream.rdbuf(m_buff); }
};
} // namespace

// https://github.com/catchorg/Catch2/issues/1813#issuecomment-563762230
void exit(int status) { throw SystemExit(status); }

namespace {

enum class ExitCondition {
  no_exception,
  expected_exit_code,
  unexpected_exit_code
};

ExitCondition with_options(const CmdLine::ArgVec &args, int expected_code) {
  try {
    CmdLine::CmdOptions options(args);
  } catch (const SystemExit &e) {
    return (e.m_exit_code == expected_code)
               ? ExitCondition::expected_exit_code
               : ExitCondition::unexpected_exit_code;
  }
  return ExitCondition::no_exception;
}

class CmdOptionsResult {
  ExitCondition m_condition;
  std::string m_cout;
  std::string m_cerr;

public:
  CmdOptionsResult(const CmdLine::ArgVec &args, int expected_code) {
    std::ostringstream couts;
    Redirect cout_capture(couts, std::cout);

    std::ostringstream cerrs;
    Redirect cerr_capture(cerrs, std::cerr);

    m_condition = with_options(args, expected_code);
    m_cout = couts.str();
    m_cerr = cerrs.str();
  }

  const ExitCondition condition() const { return m_condition; }

  std::string_view cout() const { return m_cout; }
  std::string_view cerr() const { return m_cerr; }

  bool cout_contains(std::string_view substr) {
    return m_cout.find(substr) != std::string::npos;
  }

  bool cerr_contains(std::string_view substr) {
    return m_cerr.find(substr) != std::string::npos;
  }
};

} // namespace

TEST_CASE("empty args") {
  auto result = with_options({}, 2);
  REQUIRE(result == ExitCondition::expected_exit_code);
}

TEST_CASE("help with '-h'") {
  auto result = CmdOptionsResult({"<test_program>", "-h"}, 0);
  CHECK(result.condition() == ExitCondition::expected_exit_code);
  CHECK(result.cout_contains("Usage:"));
}

TEST_CASE("help with '--help'") {
  auto result = CmdOptionsResult({"<test_program>", "--help"}, 0);
  CHECK(result.condition() == ExitCondition::expected_exit_code);
  CHECK(result.cout_contains("Usage:"));
}

TEST_CASE("no args") {
  auto result = CmdOptionsResult({"<test_program>"}, 1);
  CHECK(result.condition() == ExitCondition::expected_exit_code);
  CHECK(result.cerr_contains("Usage:"));
}

// TEST_CASE names must not start with bare "-" or "--".  If they do, they will
// annoy/confuse CMake's ctest.
TEST_CASE("no value for '-o'") {
  auto result = CmdOptionsResult({"<test_program>", "-o"}, 1);
  CHECK(result.condition() == ExitCondition::expected_exit_code);
  CHECK(result.cerr_contains("No output path"));
}

TEST_CASE("no value for '--output'") {
  auto result = CmdOptionsResult({"<test_program>", "--output"}, 1);
  CHECK(result.condition() == ExitCondition::expected_exit_code);
  CHECK(result.cerr_contains("No output path"));
}

TEST_CASE("no value for '--output='") {
  auto result =
      CmdOptionsResult({"<test_program>", "--output=", "a.jpg", "b.jpg"}, 1);
  CHECK(result.condition() == ExitCondition::expected_exit_code);
  CHECK(result.cerr_contains("No output path"));
}

TEST_CASE("typical usage") {
  CmdLine::CmdOptions opts({"<test_program>", "img1.png", "img2.png"});

  CHECK(opts.align());
  std::vector<std::filesystem::path> expected{
      std::filesystem::path("img1.png"), std::filesystem::path("img2.png")};
  CHECK(opts.input_images() == expected);
  CHECK(!opts.outpath().empty());
}

TEST_CASE("'--no-align' and '-o'") {
  CmdLine::CmdOptions opts({"<test_program>", "--no-align", "-o", "foo.jpg",
                            "img1.png", "img2.png"});

  CHECK(!opts.align());
  std::vector<std::filesystem::path> expected{
      std::filesystem::path("img1.png"), std::filesystem::path("img2.png")};
  CHECK(opts.input_images() == expected);
  CHECK(opts.outpath() == std::filesystem::path("foo.jpg"));
}

TEST_CASE("'--output='") {
  CmdLine::CmdOptions opts(
      {"<test_program>", "--output=bar.png", "img1.png", "img2.png"});
  CHECK(opts.align());
  std::vector<std::filesystem::path> expected{
      std::filesystem::path("img1.png"), std::filesystem::path("img2.png")};
  CHECK(opts.input_images() == expected);
  CHECK(opts.outpath() == std::filesystem::path("bar.png"));
}