#pragma once

#include <algorithm>
#include <array>
#include <string_view>

namespace neonsignal::routes {

namespace pages {
constexpr std::string_view kHome = "/";
constexpr std::string_view kIndex = "/index.html";
constexpr std::string_view kData = "/data";
constexpr std::string_view kDataHtml = "/data.html";
constexpr std::string_view kAuth = "/auth.html";
constexpr std::string_view kAbout = "/about.html";
constexpr std::string_view kBook = "/neonsignal-book.html";
constexpr std::string_view kCodex = "/codex";
constexpr std::string_view kCodexHtml = "/codex.html";
} // namespace pages

namespace api {
constexpr std::string_view kAuthLoginOptions = "/api/auth/login/options";
constexpr std::string_view kAuthLoginFinish = "/api/auth/login/finish";
constexpr std::string_view kAuthUserCheck = "/api/auth/user/check";
constexpr std::string_view kAuthUserRegister = "/api/auth/user/register";
constexpr std::string_view kAuthUserVerify = "/api/auth/user/verify";
constexpr std::string_view kAuthUserEnroll = "/api/auth/user/enroll";
constexpr std::string_view kCodexBrief = "/api/codex/brief";
constexpr std::string_view kCodexList = "/api/codex/list";
constexpr std::string_view kCodexItem = "/api/codex/item";
constexpr std::string_view kCodexImage = "/api/codex/image";
constexpr std::string_view kCodexRun = "/api/codex/run";
constexpr std::string_view kCodexRunStatus = "/api/codex/run/status";
constexpr std::string_view kCodexRunStdout = "/api/codex/run/stdout";
constexpr std::string_view kCodexRunStderr = "/api/codex/run/stderr";
constexpr std::string_view kCodexRunArtifacts = "/api/codex/run/artifacts";
constexpr std::string_view kMail = "/api/mail";
constexpr std::string_view kIncomingData = "/api/incoming_data";
constexpr std::string_view kStats = "/api/stats";
constexpr std::string_view kEvents = "/api/events";
constexpr std::string_view kCpu = "/api/cpu";
constexpr std::string_view kMemory = "/api/memory";
constexpr std::string_view kRedirectService = "/api/redirect_service";
} // namespace api

constexpr std::array<std::string_view, 19> kProtectedPaths = {
    pages::kData,
    pages::kDataHtml,
    pages::kCodex,
    pages::kCodexHtml,
    api::kStats,
    api::kEvents,
    api::kCpu,
    api::kMemory,
    api::kRedirectService,
    api::kAuthUserCheck,
    api::kCodexBrief,
    api::kCodexList,
    api::kCodexItem,
    api::kCodexImage,
    api::kCodexRun,
    api::kCodexRunStatus,
    api::kCodexRunStdout,
    api::kCodexRunStderr,
    api::kCodexRunArtifacts,
};

constexpr bool is_html_page(std::string_view path) {
  return path == pages::kHome || path == pages::kIndex || path == pages::kData ||
         path == pages::kDataHtml || path == pages::kAuth || path == pages::kAbout ||
         path == pages::kBook || path == pages::kCodex || path == pages::kCodexHtml;
}

inline bool is_protected(std::string_view path) {
  auto query = path.find('?');
  std::string_view clean = (query == std::string_view::npos) ? path : path.substr(0, query);
  return std::find(kProtectedPaths.begin(), kProtectedPaths.end(), clean) !=
         kProtectedPaths.end();
}

constexpr bool needs_spa_shell(std::string_view path) {
  return path == pages::kHome || path == pages::kIndex || path == pages::kData ||
         path == pages::kDataHtml || path == pages::kCodex || path == pages::kCodexHtml;
}

constexpr std::string_view default_document() { return pages::kIndex; }
constexpr std::string_view auth_redirect() { return pages::kHome; }

} // namespace neonsignal::routes
