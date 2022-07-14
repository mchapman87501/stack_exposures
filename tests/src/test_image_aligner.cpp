#include "image_aligner.hpp"
#include "image_loader.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

namespace {
auto dummy_image(size_t width, size_t height) {
  using namespace StackExposures;

  std::filesystem::path path("/no/such/image.jpg");
  cv::Mat img(height, width, CV_8UC3);
  return std::make_shared<ImageInfo>(path, img);
}
} // namespace

TEST_CASE("Align differing sizes") {
  StackExposures::ImageAligner aligner;

  auto img1 = dummy_image(16, 16);
  auto result1 = aligner.align(img1);
  CHECK(result1 == img1);

  auto img2 = dummy_image(32, 32);
  auto result2 = aligner.align(img2);
  // Can't align, so return the unmodified input image.
  CHECK(result2 == img2);
}

TEST_CASE("Align same size") {
  StackExposures::ImageAligner aligner;

  auto img1 = dummy_image(16, 16);
  auto result1 = aligner.align(img1);
  CHECK(result1 == img1);

  // Should return a new image, indicating successful alignment.
  // TODO verify same content.
  auto result2 = aligner.align(img1);
  CHECK(result2 != img1);
}

TEST_CASE("Unalignable") {
  // See CMakeLists.txt for TEST_DATA_DIR
  const std::string data_dir(TEST_DATA_DIR);

  auto loader = StackExposures::ImageLoader::create();

  auto img1 = loader->load_image(data_dir + "orientation1.jpg");
  auto img2 = loader->load_image(data_dir + "orientation2.jpg");

  StackExposures::ImageAligner aligner;
  auto result1 = aligner.align(img1);
  CHECK(result1 == img1);

  auto result2 = aligner.align(img2);
  // Alignment should fail, so img2 should be returned unchanged.
  CHECK(result2 == img2);
  // TODO capture and verify stderr messages.
  // https://github.com/catchorg/Catch2/blob/a243cbae52d5599ccce07b36cfe8b2e3308d4108/tests/ExtraTests/X27-CapturedStdoutInTestCaseEvents.cpp
}