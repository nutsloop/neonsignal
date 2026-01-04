#include "neonsignal/router.h++"

#include <string_view>

namespace neonsignal {

Router::Router(std::filesystem::path public_root)
    : public_root_(std::move(public_root)) {}

} // namespace neonsignal
