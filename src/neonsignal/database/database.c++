#include "neonsignal/database.h++"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>

namespace neonsignal {

namespace db {
std::string user_to_json(const User &user);
std::optional<User> user_from_json(std::string_view json);
std::string session_to_json(const Session &session);
std::optional<Session> session_from_json(std::string_view json);
std::string verification_to_json(const Verification &v);
std::optional<Verification> verification_from_json(std::string_view json);
std::string codex_to_json(const CodexRecord &record);
std::optional<CodexRecord> codex_from_json(std::string_view json);
std::string codex_run_to_json(const CodexRun &run);
std::optional<CodexRun> codex_run_from_json(std::string_view json);
} // namespace db

namespace {

std::vector<std::uint8_t> slice_to_bytes(const mdbx::slice &slice) {
  auto data = static_cast<const std::uint8_t *>(slice.data());
  return std::vector<std::uint8_t>(data, data + slice.size());
}

std::string_view slice_to_view(const mdbx::slice &slice) {
  auto data = static_cast<const char *>(slice.data());
  return std::string_view(data, slice.size());
}

std::string sha256_hex(std::span<const std::uint8_t> data) {
  std::array<std::uint8_t, 32> digest{};
  unsigned int len = 0;
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (!ctx) {
    return {};
  }
  if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
    EVP_MD_CTX_free(ctx);
    return {};
  }
  if (!data.empty()) {
    EVP_DigestUpdate(ctx, data.data(), data.size());
  }
  EVP_DigestFinal_ex(ctx, digest.data(), &len);
  EVP_MD_CTX_free(ctx);
  static const char *kHex = "0123456789abcdef";
  std::string out;
  out.reserve(len * 2);
  for (unsigned int i = 0; i < len; ++i) {
    out.push_back(kHex[(digest[i] >> 4) & 0xF]);
    out.push_back(kHex[digest[i] & 0xF]);
  }
  return out;
}

} // namespace

Database::Database(std::string_view path) : env_() {
  auto db_path = std::filesystem::path(std::string(path));
  if (db_path.has_parent_path()) {
    std::filesystem::create_directories(db_path.parent_path());
  }
  mdbx::env_managed::create_parameters create_params{};
  mdbx::env::operate_parameters operate_params{};
  operate_params.max_maps = 16;
  operate_params.reclaiming.lifo = true;
  operate_params.reclaiming.coalesce = true;
  env_ = mdbx::env_managed(std::string(path), create_params, operate_params);
  open_maps_();
}

void Database::open_maps_() {
  auto txn = env_.start_write();
  users_map_ = txn.create_map("users");
  emails_map_ = txn.create_map("emails");
  credentials_map_ = txn.create_map("credentials");
  usernames_map_ = txn.create_map("usernames");
  sessions_map_ = txn.create_map("sessions");
  verifications_map_ = txn.create_map("verifications");
  config_map_ = txn.create_map("config");
  codex_meta_map_ = txn.create_map("codex_meta");
  codex_payloads_map_ = txn.create_map("codex_payloads");
  codex_images_map_ = txn.create_map("codex_images");
  codex_runs_map_ = txn.create_map("codex_runs");
  codex_run_stdout_map_ = txn.create_map("codex_run_stdout");
  codex_run_stderr_map_ = txn.create_map("codex_run_stderr");
  codex_run_artifacts_map_ = txn.create_map("codex_run_artifacts");
  txn.commit();
}

// ─────────────────────────────────────────────────────────────────────────────
// New user registration flow
// ─────────────────────────────────────────────────────────────────────────────

std::optional<User> Database::create_user_pending(std::string_view email,
                                                  std::string_view display_name) {
  auto txn = env_.start_write();

  // Check if email already exists
  auto existing =
      txn.get(emails_map_, mdbx::slice(email.data(), email.size()), mdbx::slice::invalid());
  if (existing.is_valid()) {
    return std::nullopt; // Email already registered
  }

  // Get next user ID
  auto next_val = txn.get(config_map_, mdbx::slice("next_user_id"), mdbx::slice::invalid());
  std::uint64_t next_id = 1;
  if (next_val.is_valid()) {
    try {
      auto next_view = slice_to_view(next_val);
      std::string next_text(next_view.begin(), next_view.end());
      next_id = static_cast<std::uint64_t>(std::stoull(next_text));
    } catch (...) {
      next_id = 1;
    }
  }
  auto next_str = std::to_string(next_id + 1);
  txn.put(config_map_, mdbx::slice("next_user_id"), mdbx::slice(next_str), mdbx::put_mode::upsert);

  // Create user
  User user;
  user.id = next_id;
  user.email = std::string(email);
  user.display_name = std::string(display_name);
  user.verified = false;
  user.sign_count = 0;
  user.created_at = std::time(nullptr);
  user.last_login = 0;

  // Store user by ID
  auto id_key = std::to_string(next_id);
  auto json = db::user_to_json(user);
  txn.put(users_map_, mdbx::slice(id_key.data(), id_key.size()), mdbx::slice(json),
          mdbx::put_mode::insert_unique);

  // Store email → user_id index
  txn.put(emails_map_, mdbx::slice(email.data(), email.size()),
          mdbx::slice(id_key.data(), id_key.size()), mdbx::put_mode::insert_unique);

  txn.commit();
  return user;
}

std::optional<User> Database::find_user_by_email(std::string_view email) {
  auto txn = env_.start_read();
  auto id_slice =
      txn.get(emails_map_, mdbx::slice(email.data(), email.size()), mdbx::slice::invalid());
  if (!id_slice.is_valid()) {
    return std::nullopt;
  }
  auto id_str = slice_to_view(id_slice);
  auto user_data =
      txn.get(users_map_, mdbx::slice(id_str.data(), id_str.size()), mdbx::slice::invalid());
  if (!user_data.is_valid()) {
    return std::nullopt;
  }
  return db::user_from_json(slice_to_view(user_data));
}

std::optional<User> Database::find_user_by_id(std::uint64_t user_id) {
  auto txn = env_.start_read();
  auto id_key = std::to_string(user_id);
  auto user_data =
      txn.get(users_map_, mdbx::slice(id_key.data(), id_key.size()), mdbx::slice::invalid());
  if (!user_data.is_valid()) {
    return std::nullopt;
  }
  return db::user_from_json(slice_to_view(user_data));
}

bool Database::set_user_verified(std::uint64_t user_id) {
  auto txn = env_.start_write();
  auto id_key = std::to_string(user_id);
  auto user_data =
      txn.get(users_map_, mdbx::slice(id_key.data(), id_key.size()), mdbx::slice::invalid());
  if (!user_data.is_valid()) {
    return false;
  }
  auto user = db::user_from_json(slice_to_view(user_data));
  if (!user) {
    return false;
  }
  user->verified = true;
  auto json = db::user_to_json(*user);
  txn.put(users_map_, mdbx::slice(id_key.data(), id_key.size()), mdbx::slice(json),
          mdbx::put_mode::update);
  txn.commit();
  return true;
}

bool Database::set_user_credential(std::uint64_t user_id,
                                   std::span<const std::uint8_t> credential_id,
                                   std::span<const std::uint8_t> public_key) {
  auto txn = env_.start_write();
  auto id_key = std::to_string(user_id);
  auto user_data =
      txn.get(users_map_, mdbx::slice(id_key.data(), id_key.size()), mdbx::slice::invalid());
  if (!user_data.is_valid()) {
    return false;
  }
  auto user = db::user_from_json(slice_to_view(user_data));
  if (!user) {
    return false;
  }

  user->credential_id.assign(credential_id.begin(), credential_id.end());
  user->public_key.assign(public_key.begin(), public_key.end());

  auto json = db::user_to_json(*user);
  txn.put(users_map_, mdbx::slice(id_key.data(), id_key.size()), mdbx::slice(json),
          mdbx::put_mode::update);

  // Add credential_id → user_id index for WebAuthn login
  txn.put(credentials_map_, mdbx::slice(credential_id.data(), credential_id.size()),
          mdbx::slice(id_key.data(), id_key.size()), mdbx::put_mode::upsert);

  txn.commit();
  return true;
}

std::uint64_t Database::count_users() {
  auto txn = env_.start_read();
  auto cursor = txn.open_cursor(users_map_);
  std::uint64_t count = 0;

  for (auto it = cursor.to_first(false); it; it = cursor.to_next(false)) {
    ++count;
  }

  return count;
}

// ─────────────────────────────────────────────────────────────────────────────
// Legacy user methods (for backwards compatibility)
// ─────────────────────────────────────────────────────────────────────────────

std::optional<User> Database::create_user(std::string_view username,
                                          std::span<const std::uint8_t> credential_id,
                                          std::span<const std::uint8_t> public_key) {
  auto txn = env_.start_write();
  auto existing = txn.get(usernames_map_, mdbx::slice(username.data(), username.size()),
                          mdbx::slice::invalid());
  if (existing.is_valid()) {
    return std::nullopt;
  }
  auto next_val = txn.get(config_map_, mdbx::slice("next_user_id"), mdbx::slice::invalid());
  std::uint64_t next_id = 1;
  if (next_val.is_valid()) {
    try {
      auto next_view = slice_to_view(next_val);
      std::string next_text(next_view.begin(), next_view.end());
      next_id = static_cast<std::uint64_t>(std::stoull(next_text));
    } catch (...) {
      next_id = 1;
    }
  }
  auto next_str = std::to_string(next_id + 1);
  txn.put(config_map_, mdbx::slice("next_user_id"), mdbx::slice(next_str), mdbx::put_mode::upsert);
  User user;
  user.id = next_id;
  user.email = std::string(username); // Legacy: username stored as email
  user.display_name = std::string(username);
  user.verified = true; // Legacy users are considered verified
  user.credential_id.assign(credential_id.begin(), credential_id.end());
  user.public_key.assign(public_key.begin(), public_key.end());
  user.sign_count = 0;
  user.created_at = std::time(nullptr);
  user.last_login = 0;

  auto json = db::user_to_json(user);
  txn.put(users_map_, mdbx::slice(user.credential_id.data(), user.credential_id.size()),
          mdbx::slice(json), mdbx::put_mode::insert_unique);
  txn.put(usernames_map_, mdbx::slice(user.email.data(), user.email.size()),
          mdbx::slice(user.credential_id.data(), user.credential_id.size()),
          mdbx::put_mode::insert_unique);
  txn.commit();
  return user;
}

std::optional<User> Database::find_user_by_credential(std::span<const std::uint8_t> credential_id) {
  auto txn = env_.start_read();
  auto value = txn.get(users_map_, mdbx::slice(credential_id.data(), credential_id.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  auto user = db::user_from_json(slice_to_view(value));
  if (!user) {
    return std::nullopt;
  }
  user->credential_id.assign(credential_id.begin(), credential_id.end());
  return user;
}

std::optional<User> Database::find_user_by_username(std::string_view username) {
  std::vector<std::uint8_t> bytes;
  {
    auto txn = env_.start_read();
    auto cred = txn.get(usernames_map_, mdbx::slice(username.data(), username.size()),
                        mdbx::slice::invalid());
    if (!cred.is_valid()) {
      return std::nullopt;
    }
    bytes = slice_to_bytes(cred);
  }
  return find_user_by_credential(bytes);
}

std::vector<User> Database::list_users() {
  std::vector<User> out;
  auto txn = env_.start_read();
  auto cursor = txn.open_cursor(users_map_);
  for (auto kv = cursor.to_first(false); kv; kv = cursor.to_next(false)) {
    auto user = db::user_from_json(slice_to_view(kv.value));
    if (!user) {
      continue;
    }
    user->credential_id = slice_to_bytes(kv.key);
    out.push_back(std::move(*user));
  }
  return out;
}

bool Database::update_sign_count(std::span<const std::uint8_t> credential_id,
                                 std::uint32_t sign_count) {
  // First, find user_id by credential_id
  std::string user_id_str;
  {
    auto txn = env_.start_read();
    auto id_slice =
        txn.get(credentials_map_, mdbx::slice(credential_id.data(), credential_id.size()),
                mdbx::slice::invalid());
    if (id_slice.is_valid()) {
      user_id_str = std::string(slice_to_view(id_slice));
    }
  }

  if (user_id_str.empty()) {
    // Fall back to legacy: check old users_map keyed by credential_id
    auto txn = env_.start_write();
    auto value = txn.get(users_map_, mdbx::slice(credential_id.data(), credential_id.size()),
                         mdbx::slice::invalid());
    if (!value.is_valid()) {
      return false;
    }
    auto user = db::user_from_json(slice_to_view(value));
    if (!user) {
      return false;
    }
    user->sign_count = sign_count;
    user->last_login = std::time(nullptr);
    auto json = db::user_to_json(*user);
    txn.put(users_map_, mdbx::slice(credential_id.data(), credential_id.size()), mdbx::slice(json),
            mdbx::put_mode::update);
    txn.commit();
    return true;
  }

  // New flow: update user by user_id
  auto txn = env_.start_write();
  auto value = txn.get(users_map_, mdbx::slice(user_id_str.data(), user_id_str.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return false;
  }
  auto user = db::user_from_json(slice_to_view(value));
  if (!user) {
    return false;
  }
  user->sign_count = sign_count;
  user->last_login = std::time(nullptr);
  auto json = db::user_to_json(*user);
  txn.put(users_map_, mdbx::slice(user_id_str.data(), user_id_str.size()), mdbx::slice(json),
          mdbx::put_mode::update);
  txn.commit();
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Verification tokens
// ─────────────────────────────────────────────────────────────────────────────

bool Database::store_verification(std::span<const std::uint8_t> token_hash, std::uint64_t user_id,
                                  std::chrono::seconds ttl) {
  Verification v;
  v.user_id = user_id;
  v.expires_at = std::time(nullptr) + ttl.count();
  v.used_at = 0;

  auto json = db::verification_to_json(v);
  auto txn = env_.start_write();
  txn.put(verifications_map_, mdbx::slice(token_hash.data(), token_hash.size()), mdbx::slice(json),
          mdbx::put_mode::upsert);
  txn.commit();
  return true;
}

std::optional<Verification> Database::find_verification(std::span<const std::uint8_t> token_hash) {
  auto txn = env_.start_read();
  auto value = txn.get(verifications_map_, mdbx::slice(token_hash.data(), token_hash.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  return db::verification_from_json(slice_to_view(value));
}

bool Database::mark_verification_used(std::span<const std::uint8_t> token_hash) {
  auto txn = env_.start_write();
  auto value = txn.get(verifications_map_, mdbx::slice(token_hash.data(), token_hash.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return false;
  }
  auto v = db::verification_from_json(slice_to_view(value));
  if (!v) {
    return false;
  }
  v->used_at = std::time(nullptr);
  auto json = db::verification_to_json(*v);
  txn.put(verifications_map_, mdbx::slice(token_hash.data(), token_hash.size()), mdbx::slice(json),
          mdbx::put_mode::update);
  txn.commit();
  return true;
}

std::size_t Database::cleanup_expired_verifications() {
  std::size_t removed = 0;
  auto txn = env_.start_write();
  auto cursor = txn.open_cursor(verifications_map_);
  auto now = std::time(nullptr);
  for (auto kv = cursor.to_first(false); kv; kv = cursor.to_next(false)) {
    auto v = db::verification_from_json(slice_to_view(kv.value));
    if (!v) {
      continue;
    }
    // Remove if expired or already used
    if (v->expires_at < now || v->used_at != 0) {
      cursor.erase();
      ++removed;
    }
  }
  txn.commit();
  return removed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Session management
// ─────────────────────────────────────────────────────────────────────────────

std::string Database::create_session(std::uint64_t user_id, std::string_view user,
                                     std::string_view state, std::chrono::seconds ttl) {
  auto session_id = generate_session_id_();
  Session session;
  session.id = session_id;
  session.user_id = user_id;
  session.user = std::string(user);
  session.state = std::string(state);
  session.created_at = std::time(nullptr);
  session.expires_at = session.created_at + ttl.count();
  auto json = db::session_to_json(session);
  auto txn = env_.start_write();
  txn.put(sessions_map_, mdbx::slice(session_id.data(), session_id.size()), mdbx::slice(json),
          mdbx::put_mode::upsert);
  txn.commit();
  return session_id;
}

std::optional<Session> Database::validate_session(std::string_view session_id) {
  bool expired = false;
  std::optional<Session> session;
  {
    auto txn = env_.start_read();
    auto value = txn.get(sessions_map_, mdbx::slice(session_id.data(), session_id.size()),
                         mdbx::slice::invalid());
    if (!value.is_valid()) {
      return std::nullopt;
    }
    session = db::session_from_json(slice_to_view(value));
    if (!session) {
      return std::nullopt;
    }
    session->id = std::string(session_id);
    expired = session->expires_at < std::time(nullptr);
  }
  if (expired) {
    delete_session(session_id);
    return std::nullopt;
  }
  return session;
}

bool Database::update_session_expiry(std::string_view session_id, std::chrono::seconds ttl) {
  auto txn = env_.start_write();
  auto value = txn.get(sessions_map_, mdbx::slice(session_id.data(), session_id.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return false;
  }
  auto session = db::session_from_json(slice_to_view(value));
  if (!session) {
    return false;
  }
  session->id = std::string(session_id);
  session->expires_at = std::time(nullptr) + ttl.count();
  auto json = db::session_to_json(*session);
  txn.put(sessions_map_, mdbx::slice(session_id.data(), session_id.size()), mdbx::slice(json),
          mdbx::put_mode::upsert);
  txn.commit();
  return true;
}

bool Database::upgrade_session_state(std::string_view session_id, std::string_view new_state,
                                     std::chrono::seconds new_ttl) {
  auto txn = env_.start_write();
  auto value = txn.get(sessions_map_, mdbx::slice(session_id.data(), session_id.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return false;
  }
  auto session = db::session_from_json(slice_to_view(value));
  if (!session) {
    return false;
  }
  session->id = std::string(session_id);
  session->state = std::string(new_state);
  session->expires_at = std::time(nullptr) + new_ttl.count();
  auto json = db::session_to_json(*session);
  txn.put(sessions_map_, mdbx::slice(session_id.data(), session_id.size()), mdbx::slice(json),
          mdbx::put_mode::upsert);
  txn.commit();
  return true;
}

bool Database::delete_session(std::string_view session_id) {
  auto txn = env_.start_write();
  txn.erase(sessions_map_, mdbx::slice(session_id.data(), session_id.size()));
  txn.commit();
  return true;
}

std::size_t Database::cleanup_expired_sessions() {
  std::size_t removed = 0;
  auto txn = env_.start_write();
  auto cursor = txn.open_cursor(sessions_map_);
  auto now = std::time(nullptr);
  for (auto kv = cursor.to_first(false); kv; kv = cursor.to_next(false)) {
    auto session = db::session_from_json(slice_to_view(kv.value));
    if (!session) {
      continue;
    }
    if (session->expires_at < now) {
      cursor.erase();
      ++removed;
    }
  }
  txn.commit();
  return removed;
}

std::optional<CodexRecord> Database::store_codex_entry(const CodexBrief &brief,
                                                       std::string_view content_type,
                                                       std::span<const std::uint8_t> payload) {
  auto txn = env_.start_write();
  auto next_val = txn.get(config_map_, mdbx::slice("next_codex_id"), mdbx::slice::invalid());
  std::uint64_t next_id = 1;
  if (next_val.is_valid()) {
    try {
      auto next_view = slice_to_view(next_val);
      std::string next_text(next_view.begin(), next_view.end());
      next_id = static_cast<std::uint64_t>(std::stoull(next_text));
    } catch (...) {
      next_id = 1;
    }
  }
  auto next_str = std::to_string(next_id + 1);
  txn.put(config_map_, mdbx::slice("next_codex_id"), mdbx::slice(next_str), mdbx::put_mode::upsert);

  CodexRecord record;
  record.id = std::to_string(next_id);
  record.content_type = std::string(content_type);
  record.size = payload.size();
  record.created_at = std::time(nullptr);
  record.sha256 = sha256_hex(payload);
  record.title = brief.title;
  record.meta_tags = brief.meta_tags;
  record.description = brief.description;
  record.file_refs = brief.file_refs;
  record.image_name = brief.image_name;
  record.image_type = brief.image_type;
  record.image_alt = brief.image_alt;
  record.image_meta = brief.image_meta;
  record.image_size = brief.image_bytes.size();

  auto meta_json = db::codex_to_json(record);
  txn.put(codex_meta_map_, mdbx::slice(record.id.data(), record.id.size()), mdbx::slice(meta_json),
          mdbx::put_mode::upsert);
  if (!payload.empty()) {
    txn.put(codex_payloads_map_, mdbx::slice(record.id.data(), record.id.size()),
            mdbx::slice(payload.data(), payload.size()), mdbx::put_mode::upsert);
  } else {
    txn.erase(codex_payloads_map_, mdbx::slice(record.id.data(), record.id.size()));
  }
  if (!brief.image_bytes.empty()) {
    txn.put(codex_images_map_, mdbx::slice(record.id.data(), record.id.size()),
            mdbx::slice(brief.image_bytes.data(), brief.image_bytes.size()),
            mdbx::put_mode::upsert);
  } else {
    txn.erase(codex_images_map_, mdbx::slice(record.id.data(), record.id.size()));
  }
  txn.commit();
  return record;
}

std::optional<CodexRecord> Database::fetch_codex_record(std::string_view id) {
  auto txn = env_.start_read();
  auto value = txn.get(codex_meta_map_, mdbx::slice(id.data(), id.size()), mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  auto record = db::codex_from_json(slice_to_view(value));
  if (!record) {
    return std::nullopt;
  }
  record->id = std::string(id);
  return record;
}

std::optional<std::vector<std::uint8_t>> Database::fetch_codex_payload(std::string_view id) {
  auto txn = env_.start_read();
  auto value =
      txn.get(codex_payloads_map_, mdbx::slice(id.data(), id.size()), mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  return slice_to_bytes(value);
}

std::optional<std::vector<std::uint8_t>> Database::fetch_codex_image(std::string_view id) {
  auto txn = env_.start_read();
  auto value =
      txn.get(codex_images_map_, mdbx::slice(id.data(), id.size()), mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  return slice_to_bytes(value);
}

std::vector<CodexRecord> Database::list_codex(std::size_t limit) {
  std::vector<CodexRecord> out;
  auto txn = env_.start_read();
  auto cursor = txn.open_cursor(codex_meta_map_);
  for (auto kv = cursor.to_first(false); kv; kv = cursor.to_next(false)) {
    auto record = db::codex_from_json(slice_to_view(kv.value));
    if (!record) {
      continue;
    }
    record->id = std::string(slice_to_view(kv.key));
    out.push_back(std::move(*record));
  }
  std::sort(out.begin(), out.end(),
            [](const CodexRecord &a, const CodexRecord &b) { return a.created_at > b.created_at; });
  if (out.size() > limit) {
    out.resize(limit);
  }
  return out;
}

std::optional<CodexRun> Database::create_codex_run(std::string_view brief_id,
                                                   std::string_view cmdline) {
  auto txn = env_.start_write();
  auto next_val = txn.get(config_map_, mdbx::slice("next_codex_run_id"), mdbx::slice::invalid());
  std::uint64_t next_id = 1;
  if (next_val.is_valid()) {
    try {
      auto next_view = slice_to_view(next_val);
      std::string next_text(next_view.begin(), next_view.end());
      next_id = static_cast<std::uint64_t>(std::stoull(next_text));
    } catch (...) {
      next_id = 1;
    }
  }
  auto next_str = std::to_string(next_id + 1);
  txn.put(config_map_, mdbx::slice("next_codex_run_id"), mdbx::slice(next_str),
          mdbx::put_mode::upsert);

  CodexRun run;
  run.id = std::to_string(next_id);
  run.brief_id = std::string(brief_id);
  run.status = "queued";
  run.message = "";
  run.cmdline = std::string(cmdline);
  run.last_message = "";
  run.created_at = std::time(nullptr);
  run.started_at = 0;
  run.finished_at = 0;
  run.exit_code = 0;
  run.duration_ms = 0;
  run.stdout_bytes = 0;
  run.stderr_bytes = 0;
  run.artifact_count = 0;

  auto json = db::codex_run_to_json(run);
  txn.put(codex_runs_map_, mdbx::slice(run.id.data(), run.id.size()), mdbx::slice(json),
          mdbx::put_mode::insert_unique);
  txn.commit();
  return run;
}

bool Database::update_codex_run(const CodexRun &run) {
  auto json = db::codex_run_to_json(run);
  auto txn = env_.start_write();
  txn.put(codex_runs_map_, mdbx::slice(run.id.data(), run.id.size()), mdbx::slice(json),
          mdbx::put_mode::upsert);
  txn.commit();
  return true;
}

std::optional<CodexRun> Database::fetch_codex_run(std::string_view run_id) {
  auto txn = env_.start_read();
  auto value =
      txn.get(codex_runs_map_, mdbx::slice(run_id.data(), run_id.size()), mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  return db::codex_run_from_json(slice_to_view(value));
}

bool Database::store_codex_run_streams(std::string_view run_id,
                                       std::span<const std::uint8_t> stdout_bytes,
                                       std::span<const std::uint8_t> stderr_bytes) {
  auto txn = env_.start_write();
  if (!stdout_bytes.empty()) {
    txn.put(codex_run_stdout_map_, mdbx::slice(run_id.data(), run_id.size()),
            mdbx::slice(stdout_bytes.data(), stdout_bytes.size()), mdbx::put_mode::upsert);
  } else {
    txn.erase(codex_run_stdout_map_, mdbx::slice(run_id.data(), run_id.size()));
  }
  if (!stderr_bytes.empty()) {
    txn.put(codex_run_stderr_map_, mdbx::slice(run_id.data(), run_id.size()),
            mdbx::slice(stderr_bytes.data(), stderr_bytes.size()), mdbx::put_mode::upsert);
  } else {
    txn.erase(codex_run_stderr_map_, mdbx::slice(run_id.data(), run_id.size()));
  }
  txn.commit();
  return true;
}

std::optional<std::vector<std::uint8_t>> Database::fetch_codex_run_stdout(std::string_view run_id) {
  auto txn = env_.start_read();
  auto value = txn.get(codex_run_stdout_map_, mdbx::slice(run_id.data(), run_id.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  return slice_to_bytes(value);
}

std::optional<std::vector<std::uint8_t>> Database::fetch_codex_run_stderr(std::string_view run_id) {
  auto txn = env_.start_read();
  auto value = txn.get(codex_run_stderr_map_, mdbx::slice(run_id.data(), run_id.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  return slice_to_bytes(value);
}

bool Database::store_codex_run_artifacts(std::string_view run_id, std::string_view artifacts_json) {
  auto txn = env_.start_write();
  if (!artifacts_json.empty()) {
    txn.put(codex_run_artifacts_map_, mdbx::slice(run_id.data(), run_id.size()),
            mdbx::slice(artifacts_json.data(), artifacts_json.size()), mdbx::put_mode::upsert);
  } else {
    txn.erase(codex_run_artifacts_map_, mdbx::slice(run_id.data(), run_id.size()));
  }
  txn.commit();
  return true;
}

std::optional<std::string> Database::fetch_codex_run_artifacts(std::string_view run_id) {
  auto txn = env_.start_read();
  auto value = txn.get(codex_run_artifacts_map_, mdbx::slice(run_id.data(), run_id.size()),
                       mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  auto view = slice_to_view(value);
  return std::string(view.begin(), view.end());
}

std::optional<std::string> Database::get_config(std::string_view key) {
  auto txn = env_.start_read();
  auto value = txn.get(config_map_, mdbx::slice(key.data(), key.size()), mdbx::slice::invalid());
  if (!value.is_valid()) {
    return std::nullopt;
  }
  auto view = slice_to_view(value);
  return std::string(view.begin(), view.end());
}

bool Database::set_config(std::string_view key, std::string_view value) {
  auto txn = env_.start_write();
  txn.put(config_map_, mdbx::slice(key.data(), key.size()), mdbx::slice(value.data(), value.size()),
          mdbx::put_mode::upsert);
  txn.commit();
  return true;
}

bool Database::delete_config(std::string_view key) {
  auto txn = env_.start_write();
  txn.erase(config_map_, mdbx::slice(key.data(), key.size()));
  txn.commit();
  return true;
}

std::string Database::generate_session_id_() {
  std::array<std::uint8_t, 32> bytes{};
  RAND_bytes(bytes.data(), static_cast<int>(bytes.size()));
  static const char *kTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
  std::string out;
  out.reserve(44);
  std::size_t i = 0;
  while (i + 2 < bytes.size()) {
    std::uint32_t n = (bytes[i] << 16) | (bytes[i + 1] << 8) | bytes[i + 2];
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
    out.push_back(kTable[(n >> 6) & 63]);
    out.push_back(kTable[n & 63]);
    i += 3;
  }
  if (i + 1 == bytes.size()) {
    std::uint32_t n = (bytes[i] << 16);
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
  } else if (i + 2 == bytes.size()) {
    std::uint32_t n = (bytes[i] << 16) | (bytes[i + 1] << 8);
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
    out.push_back(kTable[(n >> 6) & 63]);
  }
  return out;
}

std::string Database::generate_verification_token_() {
  // Same format as session ID - 32 random bytes, base64url encoded
  return generate_session_id_();
}

} // namespace neonsignal
