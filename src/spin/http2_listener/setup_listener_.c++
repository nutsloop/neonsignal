#include "spin/http2_listener.h++"

#include "spin/http2_listener_helpers.h++"

namespace neonsignal {

void Http2Listener::setup_listener_() {
  listen_fd_ = make_listen_socket(config_);
}

} // namespace neonsignal
