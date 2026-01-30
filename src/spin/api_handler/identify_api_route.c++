#include "spin/http2_listener_api.h++"
#include "spin/routes.h++"

#include <string_view>

namespace neonsignal {

ApiRoute identify_api_route(std::string_view path) {
  auto query = path.find('?');
  std::string_view clean = (query == std::string_view::npos) ? path : path.substr(0, query);
  if (clean == routes::api::kAuthLoginOptions) {
    return ApiRoute::AuthLoginOptions;
  }
  if (clean == routes::api::kAuthLoginFinish) {
    return ApiRoute::AuthLoginFinish;
  }
  if (clean == routes::api::kAuthUserCheck) {
    return ApiRoute::AuthUserCheck;
  }
  if (clean == routes::api::kAuthUserRegister) {
    return ApiRoute::AuthUserRegister;
  }
  if (clean == routes::api::kAuthUserVerify) {
    return ApiRoute::AuthUserVerify;
  }
  if (clean == routes::api::kAuthUserEnroll) {
    return ApiRoute::AuthUserEnroll;
  }
  if (clean == routes::api::kCodexBrief) {
    return ApiRoute::CodexBrief;
  }
  if (clean == routes::api::kCodexList) {
    return ApiRoute::CodexList;
  }
  if (clean == routes::api::kCodexItem) {
    return ApiRoute::CodexItem;
  }
  if (clean == routes::api::kCodexImage) {
    return ApiRoute::CodexImage;
  }
  if (clean == routes::api::kCodexRun) {
    return ApiRoute::CodexRunStart;
  }
  if (clean == routes::api::kCodexRunStatus) {
    return ApiRoute::CodexRunStatus;
  }
  if (clean == routes::api::kCodexRunStdout) {
    return ApiRoute::CodexRunStdout;
  }
  if (clean == routes::api::kCodexRunStderr) {
    return ApiRoute::CodexRunStderr;
  }
  if (clean == routes::api::kCodexRunArtifacts) {
    return ApiRoute::CodexRunArtifacts;
  }
  if (clean == routes::api::kMail) {
    return ApiRoute::Mail;
  }
  if (clean == routes::api::kIncomingData) {
    return ApiRoute::IncomingData;
  }
  if (clean == routes::api::kStats) {
    return ApiRoute::Stats;
  }
  if (clean == routes::api::kEvents) {
    return ApiRoute::Events;
  }
  if (clean == routes::api::kCpu) {
    return ApiRoute::Cpu;
  }
  if (clean == routes::api::kMemory) {
    return ApiRoute::Memory;
  }
  if (clean == routes::api::kRedirectService) {
    return ApiRoute::RedirectService;
  }
  return ApiRoute::None;
}

} // namespace neonsignal
