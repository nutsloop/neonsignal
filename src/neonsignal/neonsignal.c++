#include "neonsignal/neonsignal.h++"

#include "neonsignal/cert_manager.h++"
#include "neonsignal/event_loop.h++"
#include "neonsignal/http2_listener.h++"
#include "neonsignal/router.h++"
#include "neonsignal/thread_pool.h++"

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
