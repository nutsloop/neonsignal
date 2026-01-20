#include "spin/neonsignal.h++"

#include "spin/cert_manager.h++"
#include "spin/event_loop.h++"
#include "spin/http2_listener.h++"
#include "spin/router.h++"
#include "spin/thread_pool.h++"

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <array>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>

namespace neonsignal {

Server::Server(ServerConfig config) : config_(std::move(config)) {}

Server::~Server() = default;

} // namespace neonsignal
