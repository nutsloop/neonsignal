#include "spin/http2_listener.h++"
#include "spin/event_loop.h++"

#include <openssl/ssl.h>
#include <unistd.h>

#include <mutex>

namespace neonsignal {

void Http2Listener::close_connection_(int fd) {
  auto conn = conn_manager_->get_connection(fd);
  if (!conn) {
    return;
  }

  if (conn->closed) {
    return;
  }

  conn->closed = true;
  loop_.remove_fd(fd);

  // Unsubscribe from all SSE channels
  sse_broadcaster_->unsubscribe_all(fd);

  if (conn->is_event_stream) {
    --event_clients_;
  }
  // No special count change for CPU streams; just close cleanly.
  SSL_shutdown(conn->ssl.get());
  close(fd);

  conn_manager_->remove_connection(fd);
}

} // namespace neonsignal
