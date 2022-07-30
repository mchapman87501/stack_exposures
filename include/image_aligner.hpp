#pragma once

#include "image_info.hpp"

namespace StackExposures {
struct ImageAligner {
  void align(const cv::Mat &ref, const cv::Mat &to_align, cv::Mat &aligned);
};
} // namespace StackExposures
