#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace janus::detail {
class ByteCursor {
  public:
    enum class Error { OUT_OF_RANGE };

  private:
    std::span<const uint8_t> data_;
    size_t offset_;

  public:
    ByteCursor(std::span<const uint8_t> bytes) : data_(bytes), offset_(0) {}

    size_t data_size() { return data_.size(); }

    size_t current_position() { return offset_; }

    std::expected<uint8_t, Error> read_u8() {
        if (offset_ + 1 > data_.size())
            return std::unexpected<Error>(Error::OUT_OF_RANGE);
        return data_[offset_++];
    }

    std::expected<uint16_t, Error> read_u16() {
        if (offset_ + 2 > data_.size())
            return std::unexpected<Error>(Error::OUT_OF_RANGE);

        uint8_t first_byte = read_u8().value();
        uint8_t second_byte = read_u8().value();

        return static_cast<uint16_t>((static_cast<uint16_t>(first_byte) << 8) | second_byte);
    }

    std::expected<uint32_t, Error> read_u24() {
        if (offset_ + 3 > data_.size())
            return std::unexpected<Error>(Error::OUT_OF_RANGE);

        uint16_t two_bytes = read_u16().value();
        uint8_t third_byte = read_u8().value();

        return (static_cast<uint32_t>(two_bytes) << 8) | static_cast<uint32_t>(third_byte);
    }

    std::expected<std::string_view, Error> read_uN(size_t n) {
        if (offset_ + n > data_.size())
            return std::unexpected<Error>(Error::OUT_OF_RANGE);

        auto slice = data_.subspan(offset_, n);
        offset_ += n;

        std::string_view domain(reinterpret_cast<const char*>(slice.data()), n);
        return domain;
    }

    bool skip(size_t n) {
        if (offset_ + n > data_.size())
            return false;

        offset_ += n;
        return true;
    }
};
} // namespace janus::detail