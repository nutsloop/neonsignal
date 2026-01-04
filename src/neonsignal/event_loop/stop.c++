#include "neonsignal/event_loop.h++"

namespace neonsignal {

void EventLoop::stop() { running_.store(false); }

} // namespace neonsignal
