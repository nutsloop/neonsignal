#include "spin/mail_cookie_store.h++"

#include <openssl/rand.h>
#include <openssl/sha.h>

#include <format>

namespace neonsignal {

MailCookieStore::MailCookieStore(const MailConfig& config) : config_(config) {}

std::string MailCookieStore::generate_sha256_code() {
  unsigned char random_bytes[32];
  if (RAND_bytes(random_bytes, static_cast<int>(sizeof(random_bytes))) != 1) {
    return {};
  }

  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256(random_bytes, sizeof(random_bytes), hash);

  std::string result;
  result.reserve(SHA256_DIGEST_LENGTH * 2);
  for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    result += std::format("{:02x}", hash[i]);
  }
  return result;
}

std::string MailCookieStore::generate_and_store(const std::string& client_ip) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto code = generate_sha256_code();
  if (code.empty()) {
    return {};
  }
  auto expires_at = std::chrono::steady_clock::now() + config_.cookie_lifespan;

  store_[client_ip] = CookieEntry{code, expires_at};
  return code;
}

bool MailCookieStore::validate(const std::string& client_ip, const std::string& code) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = store_.find(client_ip);
  if (it == store_.end()) {
    return false;
  }

  auto now = std::chrono::steady_clock::now();
  if (now > it->second.expires_at) {
    store_.erase(it);
    return false;
  }

  return it->second.code == code;
}

void MailCookieStore::cleanup_expired() {
  std::lock_guard<std::mutex> lock(mutex_);
  auto now = std::chrono::steady_clock::now();

  for (auto it = store_.begin(); it != store_.end();) {
    if (now > it->second.expires_at) {
      it = store_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace neonsignal
