#include "image_info.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <iostream>

namespace {
auto ctor1() {
  using namespace StackExposures;

  std::filesystem::path path("/no/such/image.jpg");
  cv::Mat img;
  return std::make_shared<ImageInfo>(path, img);
}
} // namespace

TEST_CASE("ctor1") {
  auto img_info = ctor1();
  CHECK(img_info != nullptr);
  CHECK(img_info->path() == std::filesystem::path("/no/such/image.jpg"));
  CHECK(img_info->image().rows == 0);
  CHECK(img_info->image().cols == 0);
}
