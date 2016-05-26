// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <aybabtu/base64.hpp>

#include <vector>

#include <gtest/gtest.h>

TEST(test_base64, encode_decode)
{
    std::vector<uint8_t> data = {1, 2, 3, 45, 5, 6, 7, 4, 99, 32, 2, 46, 7, 64};
    auto encoded = aybabtu::base64::encode(data);
    auto result = aybabtu::base64::decode(encoded);
    EXPECT_EQ(data, result);
}
