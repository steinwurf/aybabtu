// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <aybabtu/base64.hpp>

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

static void test_encode_decode(const uint8_t* data, std::size_t size)
{
    SCOPED_TRACE(testing::Message() << "size: " << size);
    auto encoded = aybabtu::base64::encode(data, size);
    EXPECT_EQ(encoded.size(), aybabtu::base64::encode_size(size));
    auto decoded_size =
        aybabtu::base64::decode_size(encoded.data(), encoded.size());
    std::vector<uint8_t> decoded(decoded_size);
    auto written =
        aybabtu::base64::decode(encoded.data(), encoded.size(), decoded.data());
    EXPECT_EQ(written, decoded_size);
    ASSERT_EQ(decoded_size, size);
    EXPECT_EQ(0, memcmp(data, decoded.data(), size));
}

TEST(test_base64, encode_decode)
{
    {
        std::vector<uint8_t> buffer = {1};
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer = {1, 2};
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer = {1, 2, 3};
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer(100, 100);
        std::generate(buffer.begin(), buffer.end(), rand);
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer = {};
        std::generate(buffer.begin(), buffer.end(), rand);
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer(100);
        std::generate(buffer.begin(), buffer.end(), rand);
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer(1000);
        std::generate(buffer.begin(), buffer.end(), rand);
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer(10000);
        std::generate(buffer.begin(), buffer.end(), rand);
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        for (uint32_t i = 0; i < 1000; ++i)
        {
            std::vector<uint8_t> buffer(rand() % 10000);
            std::generate(buffer.begin(), buffer.end(), rand);
            test_encode_decode(buffer.data(), buffer.size());
        }
    }
}

TEST(test_base64, know_results)
{
    std::string encoding_expectation = "Z2QAH6y0AoAt2AiAAAADAIAAABgHjBlQ";
    std::vector<uint8_t> decoding_expectation = {
        0x67, 0x64, 0x00, 0x1F, 0xAC, 0xB4, 0x02, 0x80, 0x2D, 0xD8, 0x08, 0x80,
        0x00, 0x00, 0x03, 0x00, 0x80, 0x00, 0x00, 0x18, 0x07, 0x8C, 0x19, 0x50};

    auto size = aybabtu::base64::decode_size(encoding_expectation.data(),
                                             encoding_expectation.size());
    EXPECT_EQ(size, decoding_expectation.size());

    std::vector<uint8_t> decoded(size);
    aybabtu::base64::decode(encoding_expectation.data(),
                            encoding_expectation.size(), decoded.data());
    EXPECT_EQ(decoding_expectation, decoded);

    auto encoded = aybabtu::base64::encode(decoding_expectation.data(),
                                           decoding_expectation.size());
    EXPECT_EQ(encoding_expectation, encoded);
}
