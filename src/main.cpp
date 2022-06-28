#include <filesystem>
#include <iostream>

#include "aligned_image_generator.hpp"

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace StackExposures;

  AlignedImageGenerator aligner;

  for (int i = 1; i < argc; ++i) {
    filesystem::path image_path(argv[i]);
    cout << "Processing " << image_path << endl;

    // TODO get the icc profile from the first image.
    if (i == 1) {
    }

    auto result = aligner.align(image_path);
    // TODO add image data to output image.
  }
  return 0;
}