#include "neonsignal/platform_utils.h++"

#include <pthread.h>

namespace neonsignal {
namespace platform_utils {

void set_thread_name(const std::string &name) {
#ifdef __APPLE__
  constexpr std::size_t kMaxNameLen = 63;
#else
  constexpr std::size_t kMaxNameLen = 15;
#endif
  std::string truncated = name;
  if (truncated.size() > kMaxNameLen) {
    truncated.resize(kMaxNameLen);
  }

#ifdef __APPLE__
  // macOS: pthread_setname_np only sets the current thread's name
  pthread_setname_np(truncated.c_str());
#else
  // Linux: pthread_setname_np takes thread handle and name
  pthread_setname_np(pthread_self(), truncated.c_str());
#endif
}

} // namespace platform_utils
} // namespace neonsignal
