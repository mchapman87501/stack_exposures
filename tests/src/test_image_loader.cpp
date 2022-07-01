#include "image_loader.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <iostream>

namespace {
void load_non_existent() {
  using namespace StackExposures;

  ImageLoader loader;
  auto unused = loader.load_image("/no/such/image.jpg");
}
} // namespace

TEST_CASE("Load non-raw image") {
  REQUIRE_THROWS_AS(load_non_existent(), std::runtime_error);
}
