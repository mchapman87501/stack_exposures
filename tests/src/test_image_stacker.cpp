#include "image_stacker.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// TODO: DRY
namespace {
using namespace StackExposures;

auto solid_color(size_t width, size_t height, const cv::Vec3b &rgb_color) {
  return ImageInfo::from_file({}, cv::Mat(height, width, CV_8UC3, rgb_color));
}

auto rgb(uint8_t red, uint8_t green, uint8_t blue) {
  return cv::Vec3b(blue, green, red);
}

template <typename PixelType>
void check_solid_color(const cv::Mat &image, const PixelType &expected,
                       std::string_view title) {
  // How to do ranged for loops over cv::Mat?
  for (int row = 0; row < image.rows; ++row) {
    for (int col = 0; col < image.cols; ++col) {
      PixelType actual = image.at<PixelType>(row, col);
      if (actual != expected) {
        std::cerr << title << ": Color mismatch at row " << row << ", col "
                  << col << ": expected " << expected << ", got " << actual
                  << std::endl;
        REQUIRE(actual == expected);
      }
    }
  }
}

cv::Mat to_image_type(const cv::Mat &src, int dtype) {
  cv::Mat result;
  src.convertTo(result, dtype);
  return result;
}

cv::Mat to_8bit(const cv::Mat &src) { return to_image_type(src, CV_8UC3); }

cv::Mat to_stacker_format(const cv::Mat &src) {
  return to_image_type(src, CV_32FC3);
}

auto future_image(const ImageInfo::SharedPtr image) {
  auto load_async = [image]() { return image; };
  return std::async(std::launch::async, load_async);
}

} // namespace

TEST_CASE("Image Stacker") {
  using namespace StackExposures;

  auto stacker = ImageStacker::create();
  ImageInfoFutureContainer images;

  SECTION("Stack one image") {
    auto color = rgb(25, 25, 250);
    auto image_info = solid_color(4, 4, color);

    images.emplace_back(future_image(image_info));

    auto stacked = stacker->stacked_result(images, nullptr, false);
    auto result = to_8bit(stacked);
    CHECK(result.rows == 4);
    CHECK(result.cols == 4);
    check_solid_color(result, color, "Stack one image");
  }

  SECTION("Differing sizes") {
    auto color1 = rgb(25, 25, 250);
    auto image1 = solid_color(4, 4, color1);

    auto color2 = rgb(225, 225, 0);
    auto image2 = solid_color(8, 8, color2);

    images.emplace_back(future_image(image1));
    images.emplace_back(future_image(image2));

    auto result = to_8bit(stacker->stacked_result(images));
    // Center-most image wins...
    REQUIRE(result.rows == 4);
    REQUIRE(result.cols == 4);
    check_solid_color(result, color1, "Differing sizes");
  }

  SECTION("No images") {
    auto stacked = stacker->stacked_result(images);
    CHECK(stacked.empty());
  }

  SECTION("With dark image") {
    auto color = rgb(150, 150, 150);
    auto image = solid_color(4, 4, color);

    auto dark_color = rgb(0, 5, 10);
    auto dark_image = solid_color(4, 4, dark_color);

    // Expected result, no matter how many copies of image are stacked,
    // is color - dark_color.
    auto expected_color = rgb(150, 145, 140);

    for (size_t i = 0; i < 100; ++i) {
      images.emplace_back(future_image(image));
      auto result = to_8bit(stacker->stacked_result(images, dark_image, false));
      REQUIRE(result.rows == 4);
      REQUIRE(result.cols == 4);
      check_solid_color(result, expected_color,
                        "With dark image - " + std::to_string(i));
    }
  }

  SECTION("Varying color") {
    // The same varying-color image added multiple times should produce that
    // same image as the averaged result.
    const int extent = 4;
    auto cv_image = cv::Mat(extent, extent, CV_8UC3, rgb(0, 0, 0));
    for (int y = 0; y < cv_image.rows; ++y) {
      for (int x = 0; x < cv_image.cols; ++x) {
        cv_image.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, x + 8);
      }
    }

    auto image = ImageInfo::from_file({}, cv_image);
    images.emplace_back(future_image(image));
    images.emplace_back(future_image(image));

    auto result = to_8bit(stacker->stacked_result(images));

    for (int x = 0; x < result.cols; ++x) {
      const auto expected = cv::Vec3b(0, 0, x + 8);
      const auto actual = result.at<cv::Vec3b>(0, x);
      if (actual != expected) {
        // How to explicitly output diagnostics, using Catch2?
        std::cerr << "Unexpected result at x=" << x << std::endl;
      }
      REQUIRE(actual == expected);
    }
  }

  SECTION("Varying color with varying dark image") {
    // Varying color image added multiple times,
    // with varying color dark image, should produce the same as
    // the original image - the dark image.
    const int extent = 4;
    auto cv_dark = solid_color(extent, extent, rgb(0, 0, 0))->image();
    auto cv_image = solid_color(extent, extent, rgb(0, 0, 0))->image();

    for (int y = 0; y < extent; ++y) {
      for (int x = 0; x < extent; ++x) {
        const double val = x + 8;
        cv_image.at<cv::Vec3b>(y, x) = cv::Vec3b(val, val, val);

        // Try to produce one dark-subtracted color for even columns
        // and one for odd columns.
        const double dark_val = (0 == x % 2) ? val - 4 : val - 6;
        // Before stretching, image - dark should have pixel intensities of 4
        // or 2.
        cv_dark.at<cv::Vec3b>(y, x) = cv::Vec3b(dark_val, dark_val, dark_val);
      }
    }

    auto image = ImageInfo::from_file({"varying image"}, cv_image);
    auto dark = ImageInfo::from_file({"dark image"}, cv_dark);

    for (size_t i = 0; i != 4; ++i) {
      images.emplace_back(future_image(image));
    }

    auto result = to_8bit(stacker->stacked_result(images, dark));

    for (int x = 0; x < result.cols; ++x) {
      // Even columns should have minimum intensity.  Odd should have max.
      const int val = (0 == x % 2) ? 4 : 6;
      const auto expected = cv::Vec3b(val, val, val);
      const auto actual = result.at<cv::Vec3b>(0, x);
      if (actual != expected) {
        // How to explicitly output diagnostics, using Catch2?
        std::cerr << "Unexpected result at x=" << x << std::endl;
      }
      REQUIRE(actual == expected);
    }
  }
}
