#include <filesystem>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "aligned_image_generator.hpp"
#include "image_stacker.hpp"

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace StackExposures;

  AlignedImageGenerator aligner;
  ImageStacker stacker;

  for (int i = 1; i < argc; ++i) {
    filesystem::path image_path(argv[i]);
    cout << "Processing " << image_path << endl;

    // TODO get the icc profile from the first image.
    if (i == 1) {
    }

    auto result = aligner.align(image_path);
    stacker.push(result->image());
  }

  // What should be the name of the output image?
  const auto result_path(filesystem::absolute(filesystem::path("stacked.jpg")));
  cout << "Saving result to " << result_path << endl;
  cv::imwrite(result_path.c_str(), stacker.result());
  return 0;
}