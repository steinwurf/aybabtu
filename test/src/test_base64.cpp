// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <aybabtu/base64.hpp>

#include <vector>

#include <gtest/gtest.h>

static void test_encode_decode(const uint8_t* data, std::size_t size)
{
    auto encoded = aybabtu::base64::encode(data, size);
    auto decoded_size = aybabtu::base64::compute_size(encoded);
    std::vector<uint8_t> decoded(decoded_size);
    auto written = aybabtu::base64::decode(decoded.data(), encoded);
    EXPECT_EQ(written, decoded_size);
    EXPECT_EQ(decoded_size, size);
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
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer = {};
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer(100, 0);
        test_encode_decode(buffer.data(), buffer.size());
    }
    {
        std::vector<uint8_t> buffer(100, 1);
        test_encode_decode(buffer.data(), buffer.size());
    }
}

TEST(test_base64, know_results)
{
    auto encoding_expectation = "Z2QAH6y0AoAt2AiAAAADAIAAABgHjBlQ";
    std::vector<uint8_t> decoding_expectation = {
        0x67, 0x64, 0x00, 0x1F, 0xAC, 0xB4, 0x02, 0x80, 0x2D, 0xD8, 0x08, 0x80,
        0x00, 0x00, 0x03, 0x00, 0x80, 0x00, 0x00, 0x18, 0x07, 0x8C, 0x19, 0x50};

    auto size = aybabtu::base64::compute_size(encoding_expectation);
    EXPECT_EQ(size, decoding_expectation.size());

    std::vector<uint8_t> decoded(size);
    aybabtu::base64::decode(decoded.data(), encoding_expectation);
    EXPECT_EQ(decoding_expectation, decoded);

    auto encoding_result = aybabtu::base64::encode(decoding_expectation.data(),
                                                   decoding_expectation.size());
    EXPECT_EQ(encoding_expectation, encoding_result);
}
