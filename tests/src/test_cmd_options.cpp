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
} // namespace

TEST_CASE("empty args") {
  auto result = with_options({}, 2);
  REQUIRE(result == ExitCondition::expected_exit_code);
}

TEST_CASE("no args") {
  auto result = with_options({"<test_program>"}, 1);
  REQUIRE(result == ExitCondition::expected_exit_code);
}

TEST_CASE("'-o' no value") {
  // See CMakeLists.txt for a feeble attempt to verify expected usage messages.
  auto result = with_options({"<test_program>", "-o"}, 1);
  REQUIRE(result == ExitCondition::expected_exit_code);
}

// TEST_CASE names must not start with bare "-" or "--".  If they do, they will
// annoy/confuse CMake's ctest.
TEST_CASE("'--output' no value") {
  auto result = with_options({"<test_program>", "--output"}, 1);
  REQUIRE(result == ExitCondition::expected_exit_code);
}

TEST_CASE("'--output='<missing_value>") {
  auto result =
      with_options({"<test_program>", "--output=", "a.jpg", "b.jpg"}, 1);
  REQUIRE(result == ExitCondition::expected_exit_code);
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