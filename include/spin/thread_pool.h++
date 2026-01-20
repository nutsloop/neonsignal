#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace neonsignal {

class ThreadPool {
public:
  struct ServerHostPort {
    std::string host;
    int port;
  };

  explicit ThreadPool(std::size_t thread_count, const ServerHostPort& server_config);
  ~ThreadPool();

  void enqueue(std::function<void()> task);

private:
  void worker_();

  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool stop_{false};
};

} // namespace neonsignal
