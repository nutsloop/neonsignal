#include "neonsignal/webauthn.h++"

#include "neonsignal/database.h++"
#include "neonsignal/routes.h++"

#include <openssl/core_names.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/param_build.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string_view>
#include <variant>

namespace neonsignal {

namespace {

std::string base64url_encode(const std::vector<std::uint8_t> &data) {
  static const char *kTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
  std::string out;
  out.reserve(((data.size() + 2) / 3) * 4);
  std::size_t i = 0;
  while (i + 2 < data.size()) {
    std::uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
    out.push_back(kTable[(n >> 6) & 63]);
    out.push_back(kTable[n & 63]);
    i += 3;
  }
  if (i + 1 == data.size()) {
    std::uint32_t n = (data[i] << 16);
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
  } else if (i + 2 == data.size()) {
    std::uint32_t n = (data[i] << 16) | (data[i + 1] << 8);
    out.push_back(kTable[(n >> 18) & 63]);
    out.push_back(kTable[(n >> 12) & 63]);
    out.push_back(kTable[(n >> 6) & 63]);
  }
  return out;
}

std::vector<std::uint8_t> base64url_decode(std::string_view input) {
  std::string s(input);
  std::replace(s.begin(), s.end(), '-', '+');
  std::replace(s.begin(), s.end(), '_', '/');
  while (s.size() % 4 != 0) {
    s.push_back('=');
  }
  int pad = 0;
  if (!s.empty() && s[s.size() - 1] == '=') ++pad;
  if (s.size() > 1 && s[s.size() - 2] == '=') ++pad;
  std::vector<unsigned char> buf((s.size() / 4) * 3 + 1);
  int len = EVP_DecodeBlock(buf.data(), reinterpret_cast<const unsigned char *>(s.data()),
                            static_cast<int>(s.size()));
  if (len < 0) {
    return {};
  }
  if (pad > 0 && len >= pad) {
    len -= pad;
  }
  return std::vector<std::uint8_t>(buf.begin(), buf.begin() + len);
}

bool secure_random(std::vector<std::uint8_t> &buf) {
  return RAND_bytes(buf.data(), static_cast<int>(buf.size())) == 1;
}

std::string extract_json_string(std::string_view json, std::string_view key) {
  auto pos = json.find("\"" + std::string(key) + "\"");
  if (pos == std::string_view::npos) {
    return {};
  }
  pos = json.find(':', pos);
  if (pos == std::string_view::npos) {
    return {};
  }
  pos = json.find('"', pos);
  if (pos == std::string_view::npos) {
    return {};
  }
  std::size_t end = json.find('"', pos + 1);
  if (end == std::string_view::npos) {
    return {};
  }
  return std::string(json.substr(pos + 1, end - pos - 1));
}

std::vector<std::uint8_t> sha256(std::string_view s) {
  std::vector<std::uint8_t> out(SHA256_DIGEST_LENGTH);
  SHA256(reinterpret_cast<const unsigned char *>(s.data()), s.size(), out.data());
  return out;
}

// Minimal CBOR decoder for the attestationObject/authData/public key path.
class CborDecoder {
public:
  explicit CborDecoder(const std::vector<std::uint8_t> &buf) : buf_(buf) {}

  std::unordered_map<std::string, std::vector<std::uint8_t>> decode_map_bytes() {
    std::unordered_map<std::string, std::vector<std::uint8_t>> out;
    std::size_t off = 0;
    auto len = read_len(0xA0, off);
    if (!len) return out;
    for (std::size_t i = 0; i < *len; ++i) {
      std::string key = read_text(off);
      if (key.empty()) return {};
      auto val = read_bytes(off);
      if (val.empty()) return {};
      out.emplace(std::move(key), std::move(val));
    }
    return out;
  }

  std::optional<std::vector<std::uint8_t>> find_bytes_field(std::string_view wanted_key) const {
    std::size_t off = 0;
    auto len = read_len(0xA0, off);
    if (!len) return std::nullopt;
    for (std::size_t i = 0; i < *len; ++i) {
      std::string key = read_text(off);
      if (key.empty()) return std::nullopt;
      if (key == wanted_key) {
        auto val = read_bytes(off);
        if (val.empty()) return std::nullopt;
        return val;
      }
      skip(buf_, off);
    }
    return std::nullopt;
  }

  struct CoseKey {
    int kty{0};
    int alg{0};
    int crv{0};
    std::vector<std::uint8_t> x;
    std::vector<std::uint8_t> y;
  };

  std::optional<CoseKey> decode_cose_key(const std::vector<std::uint8_t> &buf) {
    std::size_t off = 0;
    auto len = read_len_from(0xA0, buf, off);
    if (!len) return std::nullopt;
    CoseKey k;
    for (std::size_t i = 0; i < *len; ++i) {
      int key = read_int(buf, off);
      if (off >= buf.size()) return std::nullopt;
      if (key == 1) {
        k.kty = read_int(buf, off);
      } else if (key == 3) {
        k.alg = read_int(buf, off);
      } else if (key == -1) {
        k.crv = read_int(buf, off);
      } else if (key == -2) {
        k.x = read_bytes_from(buf, off);
      } else if (key == -3) {
        k.y = read_bytes_from(buf, off);
      } else {
        skip(buf, off);
      }
    }
    return k;
  }

private:
  std::optional<std::size_t> read_len(std::uint8_t major_mask, std::size_t &off) const {
    return read_len_from(major_mask, buf_, off);
  }

  std::optional<std::size_t> read_len_from(std::uint8_t major_mask,
                                           const std::vector<std::uint8_t> &b,
                                           std::size_t &off) const {
    if (off >= b.size()) return std::nullopt;
    std::uint8_t ib = b[off++];
    if ((ib & 0xE0) != major_mask) return std::nullopt;
    std::uint8_t ai = ib & 0x1F;
    if (ai < 24) {
      return ai;
    } else if (ai == 24) {
      if (off >= b.size()) return std::nullopt;
      return b[off++];
    } else if (ai == 25) {
      if (off + 1 >= b.size()) return std::nullopt;
      std::size_t v = (b[off] << 8) | b[off + 1];
      off += 2;
      return v;
    }
    return std::nullopt;
  }

  std::string read_text(std::size_t &off) const {
    auto len = read_len(0x60, off);
    if (!len || off + *len > buf_.size()) return {};
    std::string out(reinterpret_cast<const char *>(buf_.data() + off), *len);
    off += *len;
    return out;
  }

  std::vector<std::uint8_t> read_bytes(std::size_t &off) const {
    return read_bytes_from(buf_, off);
  }

  std::vector<std::uint8_t> read_bytes_from(const std::vector<std::uint8_t> &b,
                                            std::size_t &off) const {
    auto len = read_len_from(0x40, b, off);
    if (!len) return {};
    std::size_t byte_len = *len;
    if (off + byte_len > b.size()) return {};
    // Pre-allocate and copy to avoid GCC false positive from inlined iterator construction
    std::vector<std::uint8_t> out;
    out.reserve(byte_len);
    out.assign(b.data() + off, b.data() + off + byte_len);
    off += byte_len;
    return out;
  }

  int read_int(const std::vector<std::uint8_t> &b, std::size_t &off) const {
    if (off >= b.size()) return 0;
    std::uint8_t ib = b[off];
    if ((ib & 0xE0) == 0x00) { // unsigned
      auto len = read_len_from(0x00, b, off);
      return len ? static_cast<int>(*len) : 0;
    } else if ((ib & 0xE0) == 0x20) { // negative
      auto len = read_len_from(0x20, b, off);
      return len ? -1 - static_cast<int>(*len) : 0;
    }
    off++;
    return 0;
  }

  void skip(const std::vector<std::uint8_t> &b, std::size_t &off) const {
    if (off >= b.size()) return;
    std::uint8_t ib = b[off];
    std::uint8_t major = ib & 0xE0;
    if (major == 0x40 || major == 0x60 || major == 0x80 || major == 0xA0) {
      auto len = read_len_from(major, b, off);
      if (!len) return;
      off += *len;
    } else {
      off++;
    }
  }

  const std::vector<std::uint8_t> &buf_;
};

} // namespace

WebAuthnManager::WebAuthnManager(std::string rp_id, std::string origin, Database& db)
    : rp_id_(std::move(rp_id)), origin_(std::move(origin)), db_(db) {
}

bool WebAuthnManager::load_credentials() {
  credentials_.clear();
  auto users = db_.list_users();
  for (const auto& user : users) {
    // Skip users without credentials (pending enrollment)
    if (!user.has_credential()) {
      continue;
    }
    WebAuthnCredential cred;
    cred.user_id = user.id;
    cred.user = user.email;
    cred.rp_id = rp_id_;
    cred.credential_id = user.credential_id;
    cred.public_key_spki = user.public_key;
    cred.sign_count = user.sign_count;
    credentials_.push_back(std::move(cred));
  }
  std::cerr << "webauthn: loaded " << credentials_.size() << " credential(s)\n";
  return true;
}

WebAuthnLoginOptions WebAuthnManager::make_login_options() {
  WebAuthnLoginOptions out;
  std::vector<std::uint8_t> challenge(32);
  secure_random(challenge);
  out.challenge = base64url_encode(challenge);

  Challenge c{out.challenge, std::chrono::steady_clock::now() + std::chrono::minutes(5)};
  challenges_[out.challenge] = c;

  std::ostringstream opts;
  opts << "{\"challenge\":\"" << out.challenge << "\",";
  opts << "\"rpId\":\"" << rp_id_ << "\",";
  opts << "\"allowCredentials\":[";
  for (std::size_t i = 0; i < credentials_.size(); ++i) {
    if (i > 0) {
      opts << ",";
    }
    opts << "{\"type\":\"public-key\",\"id\":\"" << base64url_encode(credentials_[i].credential_id)
         << "\"}";
  }
  opts << "],\"timeout\":60000}";
  out.json = opts.str();
  return out;
}

std::optional<WebAuthnCredential>
WebAuthnManager::find_credential(const std::vector<std::uint8_t> &credential_id) const {
  for (const auto &cred : credentials_) {
    if (cred.credential_id == credential_id) {
      return cred;
    }
  }
  return std::nullopt;
}

std::string WebAuthnManager::issue_session(std::uint64_t user_id, std::string_view user,
                                           std::string_view state) {
  // TTL based on state: pre_webauthn = 5 minutes, auth = 5 days
  auto ttl = (state == "pre_webauthn") ? std::chrono::minutes(5)
                                       : std::chrono::hours(24 * 5);
  return db_.create_session(user_id, user, state, ttl);
}

bool WebAuthnManager::validate_session(std::string_view session_id, std::string &user_out) {
  auto session = db_.validate_session(session_id);
  if (!session) {
    return false;
  }
  user_out = session->user;
  db_.update_session_expiry(session_id, std::chrono::hours(8));
  return true;
}

WebAuthnLoginResult WebAuthnManager::finish_login(std::string_view body) {
  WebAuthnLoginResult res;

  std::string credential_id_b64 = extract_json_string(body, "credentialId");
  std::string client_data_b64 = extract_json_string(body, "clientDataJSON");
  std::string auth_data_b64 = extract_json_string(body, "authenticatorData");
  std::string signature_b64 = extract_json_string(body, "signature");

  if (credential_id_b64.empty() || client_data_b64.empty() || auth_data_b64.empty() ||
      signature_b64.empty()) {
    res.error = "missing fields";
    return res;
  }

  auto client_data = base64url_decode(client_data_b64);
  auto auth_data = base64url_decode(auth_data_b64);
  auto signature = base64url_decode(signature_b64);
  auto credential_id = base64url_decode(credential_id_b64);

  if (auth_data.size() < 32) {
    res.error = "authenticator data too small";
    return res;
  }

  auto cred = find_credential(credential_id);
  if (!cred) {
    res.error = "credential not found";
    return res;
  }

  // Check rpIdHash.
  auto rp_hash = sha256(cred->rp_id);
  if (!std::equal(rp_hash.begin(), rp_hash.end(), auth_data.begin())) {
    res.error = "rpIdHash mismatch";
    return res;
  }

  // Check user presence flag.
  const std::uint8_t flags = auth_data[32];
  if ((flags & 0x01) == 0) {
    res.error = "user not present";
    return res;
  }

  // signCount bytes 33-36 (big endian).
  std::uint32_t sign_count = 0;
  if (auth_data.size() >= 37) {
    sign_count =
        (auth_data[33] << 24) | (auth_data[34] << 16) | (auth_data[35] << 8) | auth_data[36];
    if (sign_count < cred->sign_count) {
      res.error = "sign count regression";
      return res;
    }
  }

  // Parse clientDataJSON to verify challenge and origin.
  std::string client_data_json(client_data.begin(), client_data.end());
  std::string challenge_str = extract_json_string(client_data_json, "challenge");
  if (challenge_str.empty()) {
    res.error = "missing challenge";
    return res;
  }
  auto chal_bytes = base64url_decode(challenge_str);
  auto chal_canon = chal_bytes.empty() ? challenge_str : base64url_encode(chal_bytes);
  auto ch_it = challenges_.find(challenge_str);
  if (ch_it == challenges_.end() && !chal_canon.empty()) {
    ch_it = challenges_.find(chal_canon);
  }
  if (ch_it == challenges_.end()) {
    res.error = "unknown challenge";
    return res;
  }
  if (ch_it->second.expires_at < std::chrono::steady_clock::now()) {
    challenges_.erase(ch_it);
    res.error = "challenge expired";
    return res;
  }
  challenges_.erase(ch_it);

  std::string origin = extract_json_string(client_data_json, "origin");
  if (origin != origin_) {
    res.error = "origin mismatch";
    return res;
  }

  // Verify signature.
  auto client_hash = sha256(client_data_json);
  std::vector<std::uint8_t> signed_data;
  signed_data.reserve(auth_data.size() + client_hash.size());
  signed_data.insert(signed_data.end(), auth_data.begin(), auth_data.end());
  signed_data.insert(signed_data.end(), client_hash.begin(), client_hash.end());

  const unsigned char *p = cred->public_key_spki.data();
  EVP_PKEY *pkey = d2i_PUBKEY(nullptr, &p, static_cast<long>(cred->public_key_spki.size()));
  if (!pkey) {
    res.error = "bad public key";
    return res;
  }
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  bool verify_ok = false;
  if (EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) == 1 &&
      EVP_DigestVerify(mdctx, signature.data(), static_cast<std::size_t>(signature.size()),
                       signed_data.data(), static_cast<std::size_t>(signed_data.size())) == 1) {
    verify_ok = true;
  }
  EVP_MD_CTX_free(mdctx);
  EVP_PKEY_free(pkey);
  if (!verify_ok) {
    res.error = "signature verify failed";
    return res;
  }

  res.ok = true;
  res.user = cred->user;
  res.session_id = issue_session(cred->user_id, cred->user, "auth");
  // Update sign count and persist.
  for (auto &c : credentials_) {
    if (c.credential_id == credential_id) {
      c.sign_count = sign_count;
      db_.update_sign_count(c.credential_id, c.sign_count);
      break;
    }
  }
  return res;
}

// Legacy method - deprecated, returns empty (registration secret removed)
WebAuthnRegisterOptions WebAuthnManager::make_register_options(std::string_view /*user*/,
                                                               std::string_view /*secret*/) {
  return {};
}

WebAuthnRegisterOptions WebAuthnManager::make_register_options_for_user(std::uint64_t user_id,
                                                                        std::string_view email,
                                                                        std::string_view display_name) {
  WebAuthnRegisterOptions out;

  // Verify user exists and is verified
  auto user = db_.find_user_by_id(user_id);
  if (!user || !user->verified) {
    return out;
  }

  // User must not already have a credential
  if (user->has_credential()) {
    return out;
  }

  std::vector<std::uint8_t> challenge(32);
  secure_random(challenge);
  out.challenge = base64url_encode(challenge);

  Challenge c{out.challenge, std::chrono::steady_clock::now() + std::chrono::minutes(5)};
  challenges_[out.challenge] = c;

  // Use numeric user_id as the WebAuthn user handle
  std::vector<std::uint8_t> user_handle(8);
  for (int i = 7; i >= 0; --i) {
    user_handle[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(user_id & 0xFF);
    user_id >>= 8;
  }
  std::string user_id_b64 = base64url_encode(user_handle);

  std::ostringstream opts;
  opts << "{\"challenge\":\"" << out.challenge << "\",";
  opts << "\"rp\":{\"name\":\"neonsignal\",\"id\":\"" << rp_id_ << "\"},";
  opts << "\"user\":{\"name\":\"" << email << "\",\"displayName\":\"" << display_name << "\",\"id\":\""
       << user_id_b64 << "\"},";
  opts << "\"pubKeyCredParams\":[{\"type\":\"public-key\",\"alg\":-7}],";
  opts << "\"authenticatorSelection\":{\"userVerification\":\"preferred\"},";
  opts << "\"timeout\":60000}";
  out.json = opts.str();
  return out;
}

namespace {
std::optional<std::vector<std::uint8_t>>
credential_public_key_from_authdata(const std::vector<std::uint8_t> &auth) {
  if (auth.size() < 55) {
    return std::nullopt;
  }
  // rpIdHash(32) + flags(1) + signCount(4) = 37, then attested credential data.
  std::size_t off = 37;
  off += 16; // aaguid
  if (off + 2 > auth.size()) return std::nullopt;
  std::uint16_t cred_id_len = (auth[off] << 8) | auth[off + 1];
  off += 2;
  if (off + cred_id_len > auth.size()) return std::nullopt;
  off += cred_id_len;
  if (off >= auth.size()) return std::nullopt;
  std::vector<std::uint8_t> cpk(auth.begin() + static_cast<long>(off), auth.end());
  return cpk;
}

std::optional<std::vector<std::uint8_t>> cose_to_spki(const CborDecoder::CoseKey &k) {
  // Validate COSE key parameters:
  // kty=2 (EC2), crv=1 (P-256), alg=-7 (ES256)
  if (k.kty != 2 || k.crv != 1 || k.alg != -7 || k.x.empty() || k.y.empty()) {
    return std::nullopt;
  }

  // Ensure x and y are 32 bytes (pad with leading zeros if needed)
  auto pad_to_32 = [](const std::vector<std::uint8_t> &v) {
    if (v.size() >= 32) return v;
    std::vector<std::uint8_t> padded(32 - v.size(), 0);
    padded.insert(padded.end(), v.begin(), v.end());
    return padded;
  };
  auto x_padded = pad_to_32(k.x);
  auto y_padded = pad_to_32(k.y);

  // Build the uncompressed public key point: 0x04 || x || y
  std::vector<std::uint8_t> pub_point;
  pub_point.reserve(1 + 32 + 32);
  pub_point.push_back(0x04); // Uncompressed point indicator
  pub_point.insert(pub_point.end(), x_padded.begin(), x_padded.end());
  pub_point.insert(pub_point.end(), y_padded.begin(), y_padded.end());

  // Build OSSL_PARAM array for the EC public key
  OSSL_PARAM_BLD *bld = OSSL_PARAM_BLD_new();
  if (!bld) {
    return std::nullopt;
  }

  // Set the curve name (P-256 / prime256v1)
  if (!OSSL_PARAM_BLD_push_utf8_string(bld, OSSL_PKEY_PARAM_GROUP_NAME, "prime256v1", 0)) {
    OSSL_PARAM_BLD_free(bld);
    return std::nullopt;
  }

  // Set the public key as uncompressed EC point
  if (!OSSL_PARAM_BLD_push_octet_string(bld, OSSL_PKEY_PARAM_PUB_KEY, pub_point.data(),
                                        pub_point.size())) {
    OSSL_PARAM_BLD_free(bld);
    return std::nullopt;
  }

  OSSL_PARAM *params = OSSL_PARAM_BLD_to_param(bld);
  OSSL_PARAM_BLD_free(bld);
  if (!params) {
    return std::nullopt;
  }

  // Create EVP_PKEY_CTX for EC key
  EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr);
  if (!pctx) {
    OSSL_PARAM_free(params);
    return std::nullopt;
  }

  EVP_PKEY *pkey = nullptr;
  int rc = EVP_PKEY_fromdata_init(pctx);
  if (rc <= 0) {
    EVP_PKEY_CTX_free(pctx);
    OSSL_PARAM_free(params);
    return std::nullopt;
  }

  rc = EVP_PKEY_fromdata(pctx, &pkey, EVP_PKEY_PUBLIC_KEY, params);
  EVP_PKEY_CTX_free(pctx);
  OSSL_PARAM_free(params);

  if (rc <= 0 || !pkey) {
    if (pkey) EVP_PKEY_free(pkey);
    return std::nullopt;
  }

  // Encode public key to DER/SPKI format
  std::vector<std::uint8_t> der;
  int len = i2d_PUBKEY(pkey, nullptr);
  if (len > 0) {
    der.resize(static_cast<std::size_t>(len));
    unsigned char *p = der.data();
    i2d_PUBKEY(pkey, &p);
  }
  EVP_PKEY_free(pkey);

  if (der.empty()) {
    return std::nullopt;
  }
  return der;
}
} // namespace

// Legacy method - deprecated, returns error (registration secret removed)
WebAuthnRegisterResult WebAuthnManager::finish_register(std::string_view /*body*/,
                                                        std::string_view /*secret*/) {
  WebAuthnRegisterResult res;
  res.error = "registration disabled";
  return res;
}

WebAuthnRegisterResult WebAuthnManager::finish_register_for_user(std::string_view body,
                                                                  std::uint64_t user_id) {
  WebAuthnRegisterResult res;

  // Verify user exists and is verified
  auto user = db_.find_user_by_id(user_id);
  if (!user) {
    res.error = "user not found";
    return res;
  }
  if (!user->verified) {
    res.error = "user not verified";
    return res;
  }
  if (user->has_credential()) {
    res.error = "credential already registered";
    return res;
  }

  std::string client_data_b64 = extract_json_string(body, "clientDataJSON");
  std::string att_b64 = extract_json_string(body, "attestationObject");
  std::string cred_id_b64 = extract_json_string(body, "credentialId");

  if (client_data_b64.empty() || att_b64.empty() || cred_id_b64.empty()) {
    res.error = "missing fields";
    return res;
  }

  auto client_data = base64url_decode(client_data_b64);
  auto att_obj = base64url_decode(att_b64);
  auto cred_id = base64url_decode(cred_id_b64);

  // Parse clientDataJSON
  std::string client_data_json(client_data.begin(), client_data.end());
  std::string challenge_str = extract_json_string(client_data_json, "challenge");
  std::string origin = extract_json_string(client_data_json, "origin");
  if (challenge_str.empty() || origin.empty()) {
    res.error = "invalid clientData";
    return res;
  }
  auto chal_bytes = base64url_decode(challenge_str);
  auto chal_canon = chal_bytes.empty() ? challenge_str : base64url_encode(chal_bytes);
  auto ch_it = challenges_.find(challenge_str);
  if (ch_it == challenges_.end() && !chal_canon.empty()) {
    ch_it = challenges_.find(chal_canon);
  }
  if (ch_it == challenges_.end()) {
    res.error = "unknown challenge";
    return res;
  }
  if (ch_it->second.expires_at < std::chrono::steady_clock::now()) {
    challenges_.erase(ch_it);
    res.error = "challenge expired";
    return res;
  }
  challenges_.erase(ch_it);
  if (origin != origin_) {
    res.error = "origin mismatch";
    return res;
  }

  // Parse attestationObject -> map -> authData bytes.
  CborDecoder decoder(att_obj);
  auto auth_data_opt = decoder.find_bytes_field("authData");
  if (!auth_data_opt) {
    res.error = "missing authData";
    return res;
  }
  auto auth_data = *auth_data_opt;

  auto rp_hash = sha256(rp_id_);
  if (auth_data.size() < 37 || !std::equal(rp_hash.begin(), rp_hash.end(), auth_data.begin())) {
    res.error = "rpIdHash mismatch";
    return res;
  }
  const std::uint8_t flags = auth_data[32];
  if ((flags & 0x01) == 0 || (flags & 0x40) == 0) {
    res.error = "user presence/attested flag missing";
    return res;
  }

  auto cpk_cbor = credential_public_key_from_authdata(auth_data);
  if (!cpk_cbor) {
    res.error = "missing credential public key";
    return res;
  }
  auto cose_key = decoder.decode_cose_key(*cpk_cbor);
  if (!cose_key) {
    res.error = "invalid cose key";
    return res;
  }
  auto spki = cose_to_spki(*cose_key);
  if (!spki) {
    res.error = "cannot build public key";
    return res;
  }

  // Store credential for existing user
  if (!db_.set_user_credential(user_id, cred_id, *spki)) {
    res.error = "failed to persist credential";
    return res;
  }

  // Update in-memory credentials cache
  WebAuthnCredential cred;
  cred.user_id = user_id;
  cred.user = user->email;
  cred.rp_id = rp_id_;
  cred.credential_id = cred_id;
  cred.public_key_spki = *spki;
  cred.sign_count = 0;
  credentials_.push_back(std::move(cred));

  res.ok = true;
  return res;
}

bool WebAuthnManager::user_exists(std::string_view user) {
  return db_.find_user_by_email(user).has_value();
}

bool WebAuthnManager::is_protected_path(std::string_view path) const {
  return routes::is_protected(path);
}

} // namespace neonsignal
