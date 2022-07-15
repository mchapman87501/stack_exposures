#include "image_loader.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

TEST_CASE("Load non-existent image") {
  StackExposures::ImageLoader loader;
  REQUIRE_THROWS_AS(loader.load_image("/no/such/image.jpg"),
                    std::runtime_error);
}

TEST_CASE("Load non-raw image") {
  // See CMakeLists.txt for TEST_DATA_DIR
  const std::string data_dir(TEST_DATA_DIR);
  const std::string image_path(data_dir + "exif_extractor_missing_icc.jpg");

  StackExposures::ImageLoader loader;
  auto image_info = loader.load_image(image_path);
  CHECK(image_info->path() == std::filesystem::path(image_path));
}

TEST_CASE("Load raw image") {
  const std::string data_dir(TEST_DATA_DIR);
  const std::string image_path(data_dir + "from_filesamples_com/sample1.orf");

  StackExposures::ImageLoader loader;
  auto image_info = loader.load_image(image_path);
  CHECK(image_info->path() == std::filesystem::path(image_path));
}

// TO BE WRITTEN -- so far, I can trigger only fatal libraw errors.
// TEST_CASE("Load with libraw non-fatal error") {
// }