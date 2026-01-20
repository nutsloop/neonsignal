#include "spin/hpack_decoder.h++"

#include <cstring>

namespace neonsignal {

HpackDecoder::HpackDecoder() {
  if (nghttp2_hd_inflate_new(&inflater_) != 0) {
    inflater_ = nullptr;
  }
}

HpackDecoder::~HpackDecoder() {
  if (inflater_ != nullptr) {
    nghttp2_hd_inflate_del(inflater_);
  }
}

} // namespace neonsignal
