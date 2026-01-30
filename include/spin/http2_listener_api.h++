#pragma once

#include <string_view>

namespace neonsignal {

enum class ApiRoute {
  None,
  AuthLoginOptions,
  AuthLoginFinish,
  AuthUserCheck,
  AuthUserRegister,
  AuthUserVerify,
  AuthUserEnroll,
  CodexBrief,
  CodexList,
  CodexItem,
  CodexImage,
  CodexRunStart,
  CodexRunStatus,
  CodexRunStdout,
  CodexRunStderr,
  CodexRunArtifacts,
  IncomingData,
  Mail,
  Stats,
  Events,
  Cpu,
  Memory,
  RedirectService,
};

ApiRoute identify_api_route(std::string_view path);

} // namespace neonsignal
