#include "cmd_options.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <iostream>

namespace {
struct SystemExit : public std::runtime_error {
  const int m_exit_code;
  SystemExit(int exit_code)
      : m_exit_code(exit_code), std::runtime_error(std::to_string(exit_code)) {}
};
} // namespace

// https://github.com/catchorg/Catch2/issues/1813#issuecomment-563762230
void exit(int status) { throw SystemExit(status); }

namespace {

void no_args() { CmdLine::CmdOptions options({"<test_program>"}); }

void o_no_value() { CmdLine::CmdOptions opts({"<test_program>", "-o"}); }

void output_no_value() {
  CmdLine::CmdOptions opts({"<test_program>", "--output"});
}

void output_eq_no_value() {
  CmdLine::CmdOptions opts({"<test_program>", "--output=", "a.jpg", "b.jpg"});
}

} // namespace

TEST_CASE("empty args") {
  try {
    CmdLine::CmdOptions opts({});
  } catch (const SystemExit &err) {
    CHECK(err.m_exit_code == 2);
  }
}
TEST_CASE("no args") {
  REQUIRE_THROWS_MATCHES(no_args(), SystemExit, Catch::Matchers::Message("1"));
}

TEST_CASE("'-o' no value") {
  // See CMakeLists.txt for a feeble attempt to verify expected usage messages.
  REQUIRE_THROWS_MATCHES(o_no_value(), SystemExit,
                         Catch::Matchers::Message("1"));
}

// TEST_CASE names must not start with bare "-" or "--".  If they do, they will
// annoy/confuse CMake's ctest.
TEST_CASE("'--output' no value") {
  REQUIRE_THROWS_MATCHES(output_no_value(), SystemExit,
                         Catch::Matchers::Message("1"));
}

TEST_CASE("'--output='<missing_value>") {
  REQUIRE_THROWS_MATCHES(output_eq_no_value(), SystemExit,
                         Catch::Matchers::Message("1"));
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