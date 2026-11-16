#pragma once

#include <cstdint>
#include <expected>
#include <janus/detail/byte_cursor.hpp>
#include <span>

namespace janus::detail {
class ClientHelloParser {
  public:
    enum class Error {
        INCORRECT_CONTENT_TYPE,
        INCORRECT_HANDSHAKE_TYPE,
        INCORRECT_SERVER_NAME_TYPE,
        INCORRECT_PACKAGE_LENGTH,
        TRUNCATED_PACKET,
        SNI_NOT_FOUND,
    };

    static std::expected<std::string_view, Error> parse(std::span<const uint8_t> packet) {
        ByteCursor cursor(packet);

        // читаем тип контента (1 байт)
        auto content_type = cursor.read_u8();
        if (!content_type.has_value())
            return std::unexpected(Error::TRUNCATED_PACKET);
        // проверяем код контента
        if (content_type.value() != 0x16)
            return std::unexpected(Error::INCORRECT_CONTENT_TYPE);

        // пропускаем версию записи (2 байта)
        cursor.skip(2);

        // читаем длину записи (2 байта)
        auto record_lenght = cursor.read_u16();
        if (!record_lenght.has_value())
            return std::unexpected(Error::TRUNCATED_PACKET);
        // проверка длинны полученного пакета
        // запись + предыдущие 5 байт
        if (record_lenght.value() != cursor.data_size() - 5)
            return std::unexpected(Error::INCORRECT_PACKAGE_LENGTH);

        // читаем тип рукопожатия (1 байт)
        auto handshake_type = cursor.read_u8();
        if (!handshake_type.has_value())
            return std::unexpected(Error::TRUNCATED_PACKET);
        // проверяем тип рукопожатия
        if (handshake_type.value() != 0x01)
            return std::unexpected(Error::INCORRECT_HANDSHAKE_TYPE);

        // длина тела ClientHello (3 байта)
        auto handshake_lenght = cursor.read_u24();
        if (!handshake_lenght.has_value())
            return std::unexpected(Error::TRUNCATED_PACKET);

        // пропускаем версию протокола (2 байта)
        if (!cursor.skip(2))
            return std::unexpected(Error::TRUNCATED_PACKET);
        // пропускаем клиентские байты (32 байта)
        if (!cursor.skip(32))
            return std::unexpected(Error::TRUNCATED_PACKET);

        // чистаем и пропускаем id сессии (1 байт)
        auto session_id = cursor.read_u8();
        if (!session_id.has_value() || !cursor.skip(session_id.value()))
            return std::unexpected(Error::TRUNCATED_PACKET);

        // читаем и пропускаем cipher suites (2 байта)
        auto cipher_suites = cursor.read_u16();
        if (!cipher_suites.has_value() || !cursor.skip(cipher_suites.value()))
            return std::unexpected(Error::TRUNCATED_PACKET);

        // читаем и пропускаем методы сжатия (1 байт)
        auto compression_methods = cursor.read_u8();
        if (!compression_methods.has_value() || !cursor.skip(compression_methods.value()))
            return std::unexpected(Error::TRUNCATED_PACKET);

        // читаем длину расширений (2 байта)
        auto total_extension_lenght = cursor.read_u16();
        if (!total_extension_lenght.has_value())
            return std::unexpected(Error::TRUNCATED_PACKET);

        size_t end_extension = cursor.current_position() + total_extension_lenght.value();
        while (cursor.current_position() < end_extension) {
            // читаем тип расширения (2 байта)
            auto ext_type = cursor.read_u16();
            if (!ext_type.has_value())
                return std::unexpected(Error::TRUNCATED_PACKET);
            // читаем длину расширения (2 байта)
            auto ext_len = cursor.read_u16();
            if (!ext_len.has_value())
                return std::unexpected(Error::TRUNCATED_PACKET);

            // пропускаем, если не наш тип
            if (ext_type.value() != 0x0000) {
                if (!cursor.skip(ext_len.value()))
                    return std::unexpected(Error::TRUNCATED_PACKET);
                continue;
            }

            // пропускаем длину списка (2 байта)
            if (!cursor.skip(2))
                return std::unexpected(Error::TRUNCATED_PACKET);

            // читаем тип имени сервера (1 байт)
            auto server_name_type = cursor.read_u8();
            if (!server_name_type.has_value())
                return std::unexpected(Error::TRUNCATED_PACKET);
            if (server_name_type.value() != 0x00)
                return std::unexpected(Error::INCORRECT_SERVER_NAME_TYPE);

            // читаем длину домена (2 байта)
            auto domen_name_len = cursor.read_u16();
            if (!domen_name_len.has_value())
                return std::unexpected(Error::TRUNCATED_PACKET);

            // получаем строку домена
            auto domen = cursor.read_uN(domen_name_len.value());
            if (!domen.has_value())
                return std::unexpected(Error::TRUNCATED_PACKET);

            return domen.value();
        }

        return std::unexpected(Error::SNI_NOT_FOUND);
    }
};
} // namespace janus::detail