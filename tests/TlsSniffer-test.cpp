#include <fixtures/tls_samples.hpp>
#include <gtest/gtest.h>
#include <janus/tls_sniffer.hpp>
#include <span>

using namespace janus::test;

class ClientHelloParserTest : public ::testing::Test {
  protected:
    std::span<const uint8_t> packet;
    std::span<const uint8_t> chrome_packet;

    void SetUp() {
        packet = mydomain_client_hello;
        chrome_packet = chrome_client_hello;
    }
};

TEST_F(ClientHelloParserTest, CorrectParsing) {
    auto result = janus::TlsSniffer::extract_sni(packet);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ("mydomain.test.com", result.value());
}

TEST_F(ClientHelloParserTest, TruncatedPackageWithIncorrectLength) {
    auto result = janus::TlsSniffer::extract_sni(packet.first(200));

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(janus::detail::ClientHelloParser::Error::INCORRECT_PACKAGE_LENGTH, result.error());
}

TEST_F(ClientHelloParserTest, CorrectParsingChromeExtension) {
    auto result = janus::TlsSniffer::extract_sni(chrome_packet);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ("mytest.chrome.com", result.value());
}