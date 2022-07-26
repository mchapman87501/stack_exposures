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
  return ImageInfo::from_file(path, img);
}
} // namespace

TEST_CASE("Image Aligner") {
  StackExposures::ImageAligner aligner;

  SECTION("Align differing sizes") {
    auto img1 = dummy_image(16, 16);
    auto result1 = aligner.align(img1);
    CHECK(result1 == img1);

    auto img2 = dummy_image(32, 32);
    auto result2 = aligner.align(img2);
    // Can't align, so return null.
    CHECK(result2 == nullptr);
  }

  SECTION("Align same size") {
    auto img1 = dummy_image(16, 16);
    auto result1 = aligner.align(img1);
    CHECK(result1 == img1);

    // Should return a new image, indicating successful alignment.
    // TODO verify same content.
    auto result2 = aligner.align(img1);
    CHECK(result2 != img1);
  }

  SECTION("Unalignable") {
    // See CMakeLists.txt for TEST_DATA_DIR
    const std::string data_dir(TEST_DATA_DIR);

    StackExposures::ImageLoader loader;

    auto img1 = loader.load_image(data_dir + "orientation1.jpg");
    auto img2 = loader.load_image(data_dir + "orientation2.jpg");

    auto result1 = aligner.align(img1);
    CHECK(result1 == img1);

    auto result2 = aligner.align(img2);
    // Alignment should fail, so result2 should be null.
    CHECK(result2 == nullptr);
    // TODO capture and verify stderr messages.
  }
}
