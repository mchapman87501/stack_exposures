#include "image_stacker.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace {
auto solid_color(size_t width, size_t height, const cv::Vec3b &rgb_color) {
  return cv::Mat(height, width, CV_8UC3, rgb_color);
}

auto rgb(uint8_t red, uint8_t green, uint8_t blue) {
  return cv::Vec3b(blue, green, red);
}

auto rgb16(size_t red, size_t green, size_t blue) {
  return cv::Vec3w(blue, green, red);
}

template <typename PixelType>
void check_solid_color(const cv::Mat &image, const PixelType &expected,
                       std::string_view title) {
  // How to do ranged for loops over cv::Mat?
  for (int row = 0; row < image.rows; ++row) {
    for (int col = 0; col < image.cols; ++col) {
      PixelType actual = image.at<PixelType>(row, col);
      CHECK(actual == expected);
      if (actual != expected) {
        std::cerr << title << ": Color mismatch at row " << row << ", col "
                  << col << ": expected " << std::hex << expected << ", got "
                  << actual << std::endl;
        return;
      }
    }
  }
}
} // namespace

TEST_CASE("Image Stacker") {
  using namespace StackExposures;

  auto stacker = ImageStacker::stretch();

  SECTION("Stack one image") {
    auto color = rgb(25, 25, 250);
    auto image = solid_color(4, 4, color);
    stacker->add(image);

    auto result = stacker->result8();
    CHECK(result.rows == 4);
    CHECK(result.cols == 4);
    check_solid_color(result, color, "Stack one image");
  }

  SECTION("Stack one, 16-bit output") {
    auto color = rgb(25, 25, 250);
    auto image = solid_color(4, 4, color);
    stacker->add(image);

    auto result = stacker->result16();
    CHECK(result.rows == 4);
    CHECK(result.cols == 4);
    auto expected = rgb16(25 * 0xFF, 25 * 0xFF, 250 * 0xFF);
    check_solid_color(result, expected, "Stack one, 16-bit output");
  }

  SECTION("Differing sizes") {
    auto color1 = rgb(25, 25, 250);
    auto image1 = solid_color(4, 4, color1);

    auto color2 = rgb(225, 225, 0);
    auto image2 = solid_color(8, 8, color2);
    stacker->add(image1);

    // This image should not be added because its size does not match.
    stacker->add(image2);

    auto result = stacker->result8();
    CHECK(result.rows == 4);
    CHECK(result.cols == 4);
    check_solid_color(result, color1, "Differing sizes");
  }

  SECTION("No images") {
    auto result = stacker->result16();
    CHECK(result.empty());
  }

  SECTION("With dark image") {
    // TODO Test with something other than solid-color images.
    auto color = rgb(50, 50, 50);
    auto image = solid_color(4, 4, color);

    auto dark_color = rgb(0, 5, 10);
    auto dark_image = solid_color(4, 4, dark_color);

    // Expected result, no matter how many copies of image are stacked,
    // is color - dark_color.
    stacker->subtract(dark_image);
    auto expected_color = rgb(50, 45, 40);

    for (size_t i = 0; i < 5; ++i) {
      stacker->add(image);
      auto result = stacker->result8();
      CHECK(result.rows == 4);
      CHECK(result.cols == 4);
      check_solid_color(result, expected_color,
                        "With dark image - " + std::to_string(i));
    }
  }

  SECTION("With dark image, 16-bit output") {
    auto color = rgb(50, 50, 50);
    auto image = solid_color(4, 4, color);

    auto dark_color = rgb(0, 5, 10);
    auto dark_image = solid_color(4, 4, dark_color);

    // Expected result, no matter how many copies of image are stacked,
    // is color - dark_color.
    stacker->subtract(dark_image);

    auto expected_color = rgb16(50 * 0xFF, 45 * 0xFF, 40 * 0xFF);
    for (size_t i = 0; i < 5; ++i) {
      stacker->add(image);
      auto result = stacker->result16();
      CHECK(result.rows == 4);
      CHECK(result.cols == 4);
      check_solid_color(result, expected_color,
                        "With dark image, 16-bit output - " +
                            std::to_string(i));
    }
  }

  SECTION("First dark image wins") {
    auto color = rgb(50, 50, 50);
    auto image = solid_color(4, 4, color);

    auto dark_color1 = rgb(50, 5, 10);
    auto dark_image1 = solid_color(4, 4, dark_color1);

    auto dark_color2 = rgb(20, 20, 0);
    auto dark_image2 = solid_color(4, 4, dark_color2);

    // Expected result, no matter how many copies of image are stacked,
    // is (color - dark_color1).
    stacker->subtract(dark_image1);
    stacker->subtract(dark_image2);
    auto expected_color = rgb(0, 45, 40);

    for (size_t i = 0; i < 5; ++i) {
      stacker->add(image);
      auto result = stacker->result8();
      CHECK(result.rows == 4);
      CHECK(result.cols == 4);
      check_solid_color(result, expected_color,
                        "With dark image - " + std::to_string(i));
    }
  }

  SECTION("Varying color") {
    // An image with varying color should be brightened.
    // Make an image with white ranging across some low, non-zero intensities.
    const int extent = 4;
    auto image = cv::Mat(extent, extent, CV_8UC3, rgb(0, 0, 0));
    for (int y = 0; y < image.rows; ++y) {
      for (int x = 0; x < image.cols; ++x) {
        image.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, x + 8);
      }
    }

    stacker->add(image);
    auto result = stacker->result8();
    // Output should have pixels ranging in intensity from 0 to 255.
    const double d_val = 255.0 / double(extent - 1);
    for (int x = 0; x < result.cols; ++x) {
      const int val = int(x * d_val);
      const auto expected = cv::Vec3b(0, 0, val);
      const auto actual = result.at<cv::Vec3b>(0, x);
      if (actual != expected) {
        // How to explicitly output diagnostics, using Catch2?
        std::cerr << "Unexpected result at x=" << x << std::endl;
      }
      CHECK(actual == expected);
    }
  }

  SECTION("Varying color with varying dark image") {
    const int extent = 4;
    // A constant-color dark image should have no visible effect on a
    // brightness-stretched image.  An image that is constant
    // color after dark subtraction should be constant color w. no brightness
    // adjustments.
    // So, this setup tries to create a dark-adjusted image that has a small
    // set of distinct colors.
    auto dark = solid_color(extent, extent, rgb(0, 0, 0));
    auto image = solid_color(extent, extent, rgb(0, 0, 0));
    for (int y = 0; y < extent; ++y) {
      for (int x = 0; x < extent; ++x) {
        const double val = x + 8;
        image.at<cv::Vec3b>(y, x) = cv::Vec3b(val, val, val);

        // Try to produce one dark-subtracted color for even columns
        // and one for odd columns.
        const double dark_val = (0 == x % 2) ? val - 4 : val - 6;
        // Before stretching, image - dark should have pixel intensities of 4
        // or 2.
        dark.at<cv::Vec3b>(y, x) = cv::Vec3b(dark_val, dark_val, dark_val);
      }
    }

    stacker->add(image);
    stacker->subtract(dark);
    auto result = stacker->result8();
    for (int x = 0; x < result.cols; ++x) {
      // Even columns should have minimum intensity.  Odd should have max.
      const int val = (0 == x % 2) ? 0 : 255;
      const auto expected = cv::Vec3b(val, val, val);
      const auto actual = result.at<cv::Vec3b>(0, x);
      if (actual != expected) {
        // How to explicitly output diagnostics, using Catch2?
        std::cerr << "Unexpected result at x=" << x << std::endl;
      }
      CHECK(actual == expected);
    }
  }
}
