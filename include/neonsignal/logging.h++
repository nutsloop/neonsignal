#pragma once

namespace neonsignal {

// Install a streambuf on std::cerr that prefixes each log line with the current
// thread name (pthread name if set, otherwise thread id).
void install_thread_logging_prefix();

} // namespace neonsignal
