#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace neonsignal {

class Database;

struct WebAuthnCredential {
  std::uint64_t user_id{0};
  std::string user;
  std::string rp_id;
  std::vector<std::uint8_t> credential_id;
  std::vector<std::uint8_t> public_key_spki;
  std::uint32_t sign_count{0};
};

struct WebAuthnLoginOptions {
  std::string json;
  std::string challenge;
};

struct WebAuthnRegisterOptions {
  std::string json;
  std::string challenge;
};

struct WebAuthnRegisterResult {
  bool ok{false};
  std::string error;
};

struct WebAuthnLoginResult {
  bool ok{false};
  std::string user;
  std::string error;
  std::string session_id;
};

class WebAuthnManager {
public:
  WebAuthnManager(std::string rp_id, std::string origin, Database &db);

  bool load_credentials();

  WebAuthnLoginOptions make_login_options();
  WebAuthnLoginResult finish_login(std::string_view body);

  // New user enrollment flow
  WebAuthnRegisterOptions make_register_options_for_user(std::uint64_t user_id,
                                                         std::string_view email,
                                                         std::string_view display_name);
  WebAuthnRegisterResult finish_register_for_user(std::string_view body, std::uint64_t user_id);

  // Legacy (deprecated)
  WebAuthnRegisterOptions make_register_options(std::string_view user, std::string_view secret);
  WebAuthnRegisterResult finish_register(std::string_view body, std::string_view secret);

  bool user_exists(std::string_view user);

  bool is_protected_path(std::string_view path) const;

  bool validate_session(std::string_view session_id, std::string &user_out);

  std::string issue_session(std::uint64_t user_id, std::string_view user, std::string_view state);

private:
  struct Challenge {
    std::string value;
    std::chrono::steady_clock::time_point expires_at;
  };

  std::optional<WebAuthnCredential>
  find_credential(const std::vector<std::uint8_t> &credential_id) const;

  std::string rp_id_;
  std::string origin_;
  Database &db_;
  std::vector<WebAuthnCredential> credentials_;
  std::unordered_map<std::string, Challenge> challenges_;
};

} // namespace neonsignal
