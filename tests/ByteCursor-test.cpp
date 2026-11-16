#include <gtest/gtest.h>
#include <janus/detail/byte_cursor.hpp>
#include <vector>

class ByteCursorTest : public ::testing::Test {
  protected:
    std::vector<uint8_t> sample_bytes = {'h', 'e', 'l', 'l', 'o', '!'};
};

TEST_F(ByteCursorTest, ExceptionTest) {
    std::vector<uint8_t> bytes{'h', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
    janus::detail::ByteCursor cursor(bytes);

    try {
        // Code does not throw an exception if read operation fails.
        for (size_t i = 0; i < cursor.data_size(); ++i) {
            cursor.read_uN(4);
            cursor.read_u24();
            cursor.read_u16();
            cursor.read_u8();
        }

        // Execution does not throw an exception when going out of bounds
        cursor.skip(cursor.data_size() + 1);

        SUCCEED();
    } catch (...) {
        FAIL();
    }
}

TEST_F(ByteCursorTest, CorrectReadingU8) {
    janus::detail::ByteCursor cursor(sample_bytes);

    for (size_t i = 0; i < cursor.data_size(); ++i) {
        ASSERT_EQ(i, static_cast<size_t>(cursor.current_position()));

        auto byte = cursor.read_u8();

        ASSERT_EQ(sample_bytes[i], byte.value());
    }
}

TEST_F(ByteCursorTest, CorrectReadingU16) {
    janus::detail::ByteCursor cursor(sample_bytes);

    auto read_result = cursor.read_u16();
    ASSERT_EQ(2, cursor.current_position());
    ASSERT_EQ(('h' << 8) | 'e', read_result.value());

    read_result = cursor.read_u16();
    ASSERT_EQ(4, cursor.current_position());
    ASSERT_EQ(('l' << 8) | 'l', read_result.value());

    read_result = cursor.read_u16();
    ASSERT_EQ(6, cursor.current_position());
    ASSERT_EQ(('o' << 8) | '!', read_result.value());
}

TEST_F(ByteCursorTest, CorrectReadingU24) {
    janus::detail::ByteCursor cursor(sample_bytes);

    auto read_result = cursor.read_u24();
    ASSERT_EQ(3, cursor.current_position());
    ASSERT_EQ(('h' << 16) | ('e' << 8) | 'l', read_result.value());

    read_result = cursor.read_u24();
    ASSERT_EQ(6, cursor.current_position());
    ASSERT_EQ(('l' << 16) | ('o' << 8) | '!', read_result.value());
}

TEST_F(ByteCursorTest, StringReading) {
    janus::detail::ByteCursor cursor(sample_bytes);

    auto str = cursor.read_uN(cursor.data_size());

    ASSERT_EQ("hello!", str.value());
}

TEST_F(ByteCursorTest, CombinateReading) {
    std::vector<uint8_t> bytes{'h', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
    janus::detail::ByteCursor cursor(bytes);

    ASSERT_EQ(0, cursor.current_position());
    auto byte = cursor.read_u8();
    ASSERT_EQ(1, cursor.current_position());
    ASSERT_EQ('h', byte.value());

    auto two_bytes = cursor.read_u16();
    ASSERT_EQ(3, cursor.current_position());
    ASSERT_EQ(('e' << 8) | 'l', two_bytes.value());

    auto three_bytes = cursor.read_u24();
    ASSERT_EQ(6, cursor.current_position());
    ASSERT_EQ(('l' << 16) | ('o' << 8) | ',', three_bytes.value());

    byte = cursor.read_u8();
    ASSERT_EQ(7, cursor.current_position());
    ASSERT_EQ(' ', byte.value());

    auto str = cursor.read_uN(6);
    ASSERT_EQ("world!", str);
}

TEST_F(ByteCursorTest, CorrectSkip) {
    janus::detail::ByteCursor cursor(sample_bytes);

    ASSERT_EQ(0, cursor.current_position());

    cursor.skip(2);
    ASSERT_EQ(2, cursor.current_position());
    ASSERT_EQ('l', cursor.read_u8().value());
}

TEST_F(ByteCursorTest, OutOfRangeBySkip) {
    janus::detail::ByteCursor cursor(sample_bytes);

    cursor.skip(cursor.data_size());
    auto byte = cursor.read_u8();

    ASSERT_FALSE(byte.has_value());
    ASSERT_EQ(byte.error(), janus::detail::ByteCursor::Error::OUT_OF_RANGE);
}

TEST_F(ByteCursorTest, OutOfRangeByReadingU8) {
    janus::detail::ByteCursor cursor(sample_bytes);

    cursor.skip(cursor.data_size() - 1);
    auto byte = cursor.read_u8();
    ASSERT_EQ('!', byte.value());

    byte = cursor.read_u8();
    ASSERT_FALSE(byte.has_value());
}

TEST_F(ByteCursorTest, OutOfRangeByReadingU16) {
    janus::detail::ByteCursor cursor(sample_bytes);

    cursor.skip(3);
    auto bytes = cursor.read_u16();
    ASSERT_EQ(('l' << 8) | 'o', bytes.value());

    bytes = cursor.read_u16();
    ASSERT_FALSE(bytes.has_value());
    bytes = cursor.read_u16();
    ASSERT_FALSE(bytes.has_value());
}

TEST_F(ByteCursorTest, OutOfRangeByReadingU24) {
    janus::detail::ByteCursor cursor(sample_bytes);

    cursor.skip(2);
    auto byte = cursor.read_u24();
    ASSERT_EQ(('l' << 16) | ('l' << 8) | 'o', byte.value());

    byte = cursor.read_u24();
    ASSERT_FALSE(byte.has_value());
}

TEST_F(ByteCursorTest, OutOfRangeByReadingUn) {
    janus::detail::ByteCursor cursor(sample_bytes);

    auto str = cursor.read_uN(5);
    ASSERT_EQ("hello", str.value());

    str = cursor.read_uN(10);
    ASSERT_FALSE(str.has_value());
}