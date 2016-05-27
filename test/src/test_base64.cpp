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
