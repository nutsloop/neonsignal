#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener.h++"

#include <sys/epoll.h>

#include <mutex>

namespace neonsignal {

void Http2Listener::register_connection_(std::shared_ptr<Http2Connection> conn) {
  int fd = conn->fd;
  conn_manager_->register_connection(fd, conn);

  loop_.add_fd(fd, conn->events, [this, conn](std::uint32_t events) { handle_io_(conn, events); });
}

} // namespace neonsignal
