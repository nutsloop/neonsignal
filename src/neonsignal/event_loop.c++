#include "neonsignal/event_loop.h++"

namespace neonsignal {

EventLoop::EventLoop() : backend_(create_event_loop_backend()) {}

EventLoop::~EventLoop() {
  stop();
  if (backend_) {
    backend_->cleanup();
  }
}

} // namespace neonsignal
