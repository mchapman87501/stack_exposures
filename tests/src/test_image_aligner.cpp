#include "image_aligner.hpp"
#include "image_loader.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace {
auto solid_color(int rows, int cols, const cv::Vec3b &rgb_color) {
  return cv::Mat(rows, cols, CV_8UC3, rgb_color);
}

auto rgb(uint8_t red, uint8_t green, uint8_t blue) {
  return cv::Vec3b(blue, green, red);
}

auto image(int rows, int cols, int x_offset = 0, int y_offset = 0) {
  auto result = solid_color(rows, cols, rgb(0, 0, 0));
  // sprinkle some white spots at fixed locations.
  const auto light = rgb(255, 255, 255);

  const std::vector<cv::Point2i> points{
      {rows / 2, cols / 2}, {0, 0},        {rows - 1, cols - 1},
      {rows - 1, 0},        {0, cols - 1},
  };
  for (const auto p : points) {
    const int y = p.y + y_offset;
    const int x = p.x + x_offset;
    if ((0 <= x) && (x < cols) && (0 <= y) && (y < rows)) {
      result.at<cv::Vec3b>(y, x) = light;
    }
  }
  return result;
}

} // namespace
TEST_CASE("Image Aligner") {
  StackExposures::ImageAligner aligner;

  SECTION("Align differing sizes") {
    auto img1 = image(16, 16);
    auto img2 = image(8, 8);
    cv::Mat result;
    aligner.align(img1, img2, result);
    CHECK(result.empty());
  }

  SECTION("Align same size") {
    auto img1 = image(16, 16);
    auto img2 = image(16, 16, 1, 2);
    cv::Mat result;
    aligner.align(img1, img2, result);
    CHECK(!result.empty());
    CHECK(result.rows == 16);
    CHECK(result.cols == 16);
    // TODO verify content
  }

  SECTION("Unalignable") {
    // See CMakeLists.txt for TEST_DATA_DIR
    const std::string data_dir(TEST_DATA_DIR);

    StackExposures::ImageLoader loader;

    auto img1 = loader.load_image(data_dir + "orientation1.jpg");
    auto img2 = loader.load_image(data_dir + "orientation2.jpg");

    cv::Mat result;
    aligner.align(img1->image(), img2->image(), result);
    CHECK(result.empty());
    // TODO capture and verify stderr messages.
  }
}
