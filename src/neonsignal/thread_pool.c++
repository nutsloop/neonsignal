#include "neonsignal/thread_pool.h++"

#include <pthread.h>

#include <format>
#include <iostream>
#include <string>

namespace neonsignal {

ThreadPool::ThreadPool(std::size_t thread_count, const ServerHostPort &server_config) {
  if (thread_count == 0) {
    thread_count = 1;
  }
  threads_.reserve(thread_count);
  for (std::size_t i = 0; i < thread_count; ++i) {
    threads_.emplace_back([this, i, server_config] {
      std::string thread_name = std::format("neonsignal->({})", std::to_string(i));
      if (thread_name.size() > 15) {
        thread_name.resize(15);
      }
      pthread_setname_np(pthread_self(), thread_name.c_str());
      std::cerr << std::format("@{}:{}", server_config.host, server_config.port) << std::endl;
      worker_();
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
  }
  cv_.notify_all();
  for (auto &thread : threads_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

} // namespace neonsignal
