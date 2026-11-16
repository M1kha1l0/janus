#pragma once

#include <cstdint>
#include <expected>
#include <janus/detail/clienthello_parser.hpp>
#include <span>
#include <string_view>

namespace janus {
using SnifferError = detail::ClientHelloParser::Error;

class TlsSniffer {
  public:
    static std::expected<std::string_view, SnifferError>
    extract_sni(std::span<const uint8_t> packet) {
        return detail::ClientHelloParser::parse(packet);
    }
};
} // namespace janus