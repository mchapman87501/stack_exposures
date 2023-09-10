#include "str_util.hpp"
#include <algorithm>

namespace StrUtil {

std::string lowercase(std::string_view s) {
  std::string result(s);
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}
} // namespace StrUtil