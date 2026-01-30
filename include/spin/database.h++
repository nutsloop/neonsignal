#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <mdbx.h++>

namespace neonsignal {

struct User {
  std::uint64_t id{};
  std::string email;
  std::string display_name;
  bool verified{false};
  std::vector<std::uint8_t> credential_id;  // empty until enrolled
  std::vector<std::uint8_t> public_key;     // empty until enrolled
  std::uint32_t sign_count{};
  std::time_t created_at{};
  std::time_t last_login{};

  [[nodiscard]] bool has_credential() const { return !credential_id.empty(); }
};

struct Session {
  std::string id;
  std::uint64_t user_id{};
  std::string user;
  std::string state;  // "pre_webauthn" or "auth"
  std::time_t created_at{};
  std::time_t expires_at{};
};

struct Verification {
  std::uint64_t user_id{};
  std::time_t expires_at{};
  std::time_t used_at{};  // 0 if not used
};

struct CodexRecord {
  std::string id;
  std::string content_type;
  std::string sha256;
  std::uint64_t size{};
  std::time_t created_at{};
  std::string title;
  std::string meta_tags;
  std::string description;
  std::string file_refs;
  std::string image_name;
  std::string image_type;
  std::string image_alt;
  std::string image_meta;
  std::uint64_t image_size{};
};

struct CodexBrief {
  std::string title;
  std::string meta_tags;
  std::string description;
  std::string file_refs;
  std::string image_name;
  std::string image_type;
  std::string image_alt;
  std::string image_meta;
  std::vector<std::uint8_t> image_bytes;
};

struct CodexRun {
  std::string id;
  std::string brief_id;
  std::string status;
  std::string message;
  std::string cmdline;
  std::string last_message;
  std::time_t created_at{};
  std::time_t started_at{};
  std::time_t finished_at{};
  int exit_code{};
  std::uint64_t duration_ms{};
  std::uint64_t stdout_bytes{};
  std::uint64_t stderr_bytes{};
  std::uint64_t artifact_count{};
};

struct MailSubmission {
  std::uint64_t n{};
  std::string id;
  std::string ip;
  std::time_t date{};
  std::string form;
  std::string from;
  std::string to;
  std::string subject;
  std::string body;
  std::string status;
};

class Database {
public:
  explicit Database(std::string_view path);
  ~Database() = default;

  Database(const Database&) = delete;
  Database& operator=(const Database&) = delete;
  Database(Database&&) = default;
  Database& operator=(Database&&) = default;

  // User management - new registration flow
  std::optional<User> create_user_pending(std::string_view email, std::string_view display_name);
  std::optional<User> find_user_by_email(std::string_view email);
  std::optional<User> find_user_by_id(std::uint64_t user_id);
  bool set_user_verified(std::uint64_t user_id);
  bool set_user_credential(std::uint64_t user_id,
                           std::span<const std::uint8_t> credential_id,
                           std::span<const std::uint8_t> public_key);
  std::uint64_t count_users();

  // Legacy user methods (for WebAuthn login)
  std::optional<User> create_user(std::string_view username,
                                  std::span<const std::uint8_t> credential_id,
                                  std::span<const std::uint8_t> public_key);
  std::optional<User> find_user_by_credential(std::span<const std::uint8_t> credential_id);
  std::optional<User> find_user_by_username(std::string_view username);
  std::vector<User> list_users();
  bool update_sign_count(std::span<const std::uint8_t> credential_id, std::uint32_t sign_count);

  // Verification tokens
  bool store_verification(std::span<const std::uint8_t> token_hash,
                          std::uint64_t user_id,
                          std::chrono::seconds ttl);
  std::optional<Verification> find_verification(std::span<const std::uint8_t> token_hash);
  bool mark_verification_used(std::span<const std::uint8_t> token_hash);
  std::size_t cleanup_expired_verifications();

  // Session management
  std::string create_session(std::uint64_t user_id, std::string_view user,
                             std::string_view state, std::chrono::seconds ttl);
  std::optional<Session> validate_session(std::string_view session_id);
  bool update_session_expiry(std::string_view session_id, std::chrono::seconds ttl);
  bool upgrade_session_state(std::string_view session_id,
                             std::string_view new_state,
                             std::chrono::seconds new_ttl);
  bool delete_session(std::string_view session_id);
  std::size_t cleanup_expired_sessions();

  std::optional<CodexRecord> store_codex_entry(const CodexBrief& brief,
                                               std::string_view content_type,
                                               std::span<const std::uint8_t> payload);
  std::optional<CodexRecord> fetch_codex_record(std::string_view id);
  std::optional<std::vector<std::uint8_t>> fetch_codex_payload(std::string_view id);
  std::optional<std::vector<std::uint8_t>> fetch_codex_image(std::string_view id);
  std::vector<CodexRecord> list_codex(std::size_t limit);

  std::optional<CodexRun> create_codex_run(std::string_view brief_id,
                                           std::string_view cmdline);
  bool update_codex_run(const CodexRun& run);
  std::optional<CodexRun> fetch_codex_run(std::string_view run_id);
  bool store_codex_run_streams(std::string_view run_id,
                               std::span<const std::uint8_t> stdout_bytes,
                               std::span<const std::uint8_t> stderr_bytes);
  std::optional<std::vector<std::uint8_t>> fetch_codex_run_stdout(std::string_view run_id);
  std::optional<std::vector<std::uint8_t>> fetch_codex_run_stderr(std::string_view run_id);
  bool store_codex_run_artifacts(std::string_view run_id, std::string_view artifacts_json);
  std::optional<std::string> fetch_codex_run_artifacts(std::string_view run_id);

  bool create_mail_submission(const MailSubmission& submission);
  std::optional<MailSubmission> fetch_mail_submission(std::string_view id);
  std::optional<MailSubmission> fetch_mail_submission_by_n(std::uint64_t n);
  std::uint64_t get_next_mail_submission_n();
  bool update_mail_submission_status(std::string_view id, std::string_view status);

  std::optional<std::string> get_config(std::string_view key);
  bool set_config(std::string_view key, std::string_view value);
  bool delete_config(std::string_view key);

private:
  void open_maps_();
  static std::string generate_session_id_();
  static std::string generate_verification_token_();

  mdbx::env_managed env_;
  mdbx::map_handle users_map_;           // user_id → user JSON
  mdbx::map_handle emails_map_;          // email → user_id
  mdbx::map_handle credentials_map_;     // credential_id → user_id
  mdbx::map_handle usernames_map_;       // legacy: username → credential_id
  mdbx::map_handle sessions_map_;
  mdbx::map_handle verifications_map_;   // token_hash → verification JSON
  mdbx::map_handle config_map_;
  mdbx::map_handle codex_meta_map_;
  mdbx::map_handle codex_payloads_map_;
  mdbx::map_handle codex_images_map_;
  mdbx::map_handle codex_runs_map_;
  mdbx::map_handle codex_run_stdout_map_;
  mdbx::map_handle codex_run_stderr_map_;
  mdbx::map_handle codex_run_artifacts_map_;
  mdbx::map_handle mail_submissions_map_;
  mdbx::map_handle mail_submission_n_map_;
};

} // namespace neonsignal
