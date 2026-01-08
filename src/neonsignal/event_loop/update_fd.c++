#include "neonsignal/event_loop.h++"

namespace neonsignal {

void EventLoop::update_fd(int fd, std::uint32_t events) {
  backend_->update_fd(fd, events);
}

} // namespace neonsignal
