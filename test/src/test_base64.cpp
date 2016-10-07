// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <aybabtu/base64.hpp>

#include <vector>

#include <gtest/gtest.h>

static void test_encode_decode(const std::vector<uint8_t>& data)
{
    auto encoded = aybabtu::base64::encode(data);
    auto result = aybabtu::base64::decode(encoded);
    EXPECT_EQ(data, result);
}

TEST(test_base64, encode_decode)
{
    {
        std::vector<uint8_t> data = {1};
        test_encode_decode(data);
    }
    {
        std::vector<uint8_t> data = {1, 2};
        test_encode_decode(data);
    }
    {
        std::vector<uint8_t> data = {1, 2, 3};
        test_encode_decode(data);
    }
    {
        std::vector<uint8_t> data(100, 100);
        test_encode_decode(data);
    }
    {
        std::vector<uint8_t> data = {};
        test_encode_decode(data);
    }
    {
        std::vector<uint8_t> data(100, 0);
        test_encode_decode(data);
    }
    {
        std::vector<uint8_t> data(100, 1);
        test_encode_decode(data);
    }
}

TEST(test_base64, know_results)
{
    auto encoding_expectation = "Z2QAH6y0AoAt2AiAAAADAIAAABgHjBlQ";
    std::vector<uint8_t> decoding_expectation =
        {
            0x67, 0x64, 0x00, 0x1F, 0xAC, 0xB4, 0x02, 0x80, 0x2D, 0xD8, 0x08,
            0x80, 0x00, 0x00, 0x03, 0x00, 0x80, 0x00, 0x00, 0x18, 0x07, 0x8C,
            0x19, 0x50
        };

    auto decoding_result = aybabtu::base64::decode(encoding_expectation);
    EXPECT_EQ(decoding_expectation, decoding_result);

    auto encoding_result = aybabtu::base64::encode(decoding_expectation);
    EXPECT_EQ(encoding_expectation, encoding_result);
}
