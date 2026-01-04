#include <neonsignal/vhost.h++>

namespace neonsignal {

VHostResolver::VHostResolver(std::filesystem::path public_root)
    : public_root_(std::move(public_root)) {
  refresh();
}

} // namespace neonsignal
