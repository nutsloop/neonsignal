#ifndef NEONSIGNAL_PLATFORM_UTILS_H
#define NEONSIGNAL_PLATFORM_UTILS_H

#include <string>

namespace neonsignal {
namespace platform_utils {

// Sets the name of the current thread. Thread names are typically limited to
// 15 characters on Linux and 63 on macOS; this function will truncate longer
// names.
void set_thread_name(const std::string &name);

} // namespace platform_utils
} // namespace neonsignal

#endif // NEONSIGNAL_PLATFORM_UTILS_H
