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

void throw_on_fail(StackExposures::LibRawPtr proc, int status, const std::string& msg) {
  if (status > 0) {
    throw std::runtime_error(msg + std::strerror(status));
  }
  if (status < 0) {
    throw std::runtime_error(msg + proc->strerror(status));
  }
}


// Expect test to run in .../build/tests
std::filesystem::path ctor2_path("not_a_real_image.rw2");

auto ctor2() {
	using namespace StackExposures;

  
	LibRawPtr processor = std::make_shared<LibRaw>();
  // Try to fabricate a processed image that contains no real data, but that will not
  // crash on processor->dcraw_clear_mem.
  // Mimic internal logic of LibRaw::dcraw_make_mem_image.
  unsigned int width = 4;
  unsigned int height = 4;
  unsigned int colors = 3; // RGB
  unsigned int bytes_per_sample = 1;
  unsigned int dummy_data_size = (width * bytes_per_sample * colors) * height;
  libraw_processed_image_t *dummy = (libraw_processed_image_t *)::malloc(
    sizeof(libraw_processed_image_t) + dummy_data_size);
  dummy->height = height;
  dummy->width = width;
  dummy->colors = colors;
  dummy->bits = bytes_per_sample * 8;
  dummy->data_size = dummy_data_size;

	return std::make_shared<ImageInfo>(processor, ctor2_path, dummy);
}

auto ctor3() {
  using namespace StackExposures;
  auto dummy_img_info(ctor2());
  cv::Mat dummy_image(4, 4, CV_8UC3);
  return std::make_shared<ImageInfo>(*dummy_img_info, dummy_image);
}
} // namespace

TEST_CASE("ctor1") {
  auto img_info = ctor1();
  CHECK(img_info != nullptr);
  CHECK(img_info->path() == std::filesystem::path("/no/such/image.jpg"));
  CHECK(img_info->image().rows == 0);
  CHECK(img_info->image().cols == 0);
}

TEST_CASE("ctor2") {
  auto img_info = ctor2();
  CHECK(img_info != nullptr);
  CHECK(img_info->path() == ctor2_path);
  CHECK(img_info->image().rows > 0);
  CHECK(img_info->image().cols > 0);
  // How to ensure img_info dtor releases image memory?
}

TEST_CASE("ctor3") {
  auto img_info = ctor3();
  CHECK(img_info != nullptr);
  CHECK(img_info->path() == ctor2_path);
  CHECK(img_info->image().rows > 0);
  CHECK(img_info->image().cols > 0);
}

