#include "neonsignal/thread_pool.h++"

#include <stdexcept>
#include <utility>

namespace neonsignal {

void ThreadPool::enqueue(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stop_) {
      throw std::runtime_error("cannot enqueue on stopped ThreadPool");
    }
    tasks_.push(std::move(task));
  }
  cv_.notify_one();
}

} // namespace neonsignal
