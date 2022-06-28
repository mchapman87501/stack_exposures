#include <sstream>
#include <stdexcept>
#include <string>

#include "aligned_image_generator.hpp"

namespace {
void check(int status, const std::string &msg) {
  using namespace std;

  if (LIBRAW_FATAL_ERROR(status)) {
    ostringstream outs;
    outs << "Fatal error: " << msg;
    throw new runtime_error(outs.str());
  }
  if (LIBRAW_SUCCESS != status) {
    throw new runtime_error(msg);
  }
}
} // namespace

namespace StackExposures {
ImageInfo::UniquePtr
AlignedImageGenerator::align(const std::filesystem::path &image_path) {
  ImageInfo::UniquePtr result =
      std::make_unique<ImageInfo>(ImageInfo(image_path));

  LibRaw &processor(result->processor());

  check(processor.open_file(image_path.c_str()), "Could not open file");
  check(processor.unpack(), "Could not unpack");
  check(processor.raw2image(), "raw2image");
  check(processor.dcraw_process(), "dcraw_process"); // This is what Rawpy uses.

  // To extract image data, rawpy uses
  // dcraw_make_mem_image().
  int status;
  libraw_processed_image_t *img = processor.dcraw_make_mem_image(&status);
  check(status, "dcraw_make_mem_image");
  assert(img);
  assert(img->type == LIBRAW_IMAGE_BITMAP);

  result->set_raw_image(img);

  return result;
  // processed_image_wrapper logic goes here:
  /*
   * wrapped = processed_image_wrapper()
   * wrapped.set_data(self, img)
   * ndarr = wrapped.__array__()
   * return ndarr
   */
  /*
  cdef class processed_image_wrapper:
  cdef RawPy raw
  cdef libraw_processed_image_t* processed_image

  cdef set_data(self, RawPy raw, libraw_processed_image_t* processed_image):
      self.raw = raw
      self.processed_image = processed_image

  def __array__(self):
      cdef np.npy_intp shape[3]
      shape[0] = <np.npy_intp> self.processed_image.height
      shape[1] = <np.npy_intp> self.processed_image.width
      shape[2] = <np.npy_intp> self.processed_image.colors
      cdef np.ndarray ndarr
      ndarr = np.PyArray_SimpleNewFromData(3, shape,
                                           np.NPY_UINT8 if
  self.processed_image.bits == 8 else np.NPY_UINT16, self.processed_image.data)
      ndarr.base = <PyObject*> self
      # Python doesn't know about above assignment as it's in C-level
      Py_INCREF(self)
      return ndarr

  def __dealloc__(self):
      self.raw.p.dcraw_clear_mem(self.processed_image)

  */
}
} // namespace StackExposures
