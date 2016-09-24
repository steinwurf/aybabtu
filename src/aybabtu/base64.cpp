// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64.hpp"

#include <cassert>
#include <string>
#include <vector>
#include <cstdint>

namespace aybabtu
{
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64::encode(const std::vector<uint8_t>& buffer)
{
    return base64::encode(buffer.data(), buffer.size());
}

std::string base64::encode(const uint8_t* data, uint32_t size)
{
    std::string result;
    for (uint32_t i = 0; i < size; i += 3)
    {
        uint8_t b = (data[i] & 0xFC) >> 2;
        result += base64_chars[b];

        b = (data[i] & 0x03) << 4;
        if ((i + 1) < size)
        {
            b |= (data[i + 1] & 0xF0) >> 4;
            result += base64_chars[b];
            b = (data[i + 1] & 0x0F) << 2;
            if (i + 2 < size)
            {
                b |= (data[i + 2] & 0xC0) >> 6;
                result += base64_chars[b];
                b = data[i + 2] & 0x3F;
                result += base64_chars[b];
            }
            else
            {
                result += base64_chars[b];
                result += '=';
            }
        }
        else
        {
            result += base64_chars[b];
            result += "==";
        }
    }

    return result;
}

std::vector<uint8_t> base64::decode(const std::string& encoded_data)
{
    assert(encoded_data.length() % 4 == 0);
    std::vector<uint8_t> result;

    uint8_t b[4];
    for (uint32_t i = 0U; i < encoded_data.length(); i += 4)
    {
        b[0] = base64_chars.find(encoded_data[i]);
        b[1] = base64_chars.find(encoded_data[i + 1]);
        b[2] = base64_chars.find(encoded_data[i + 2]);
        b[3] = base64_chars.find(encoded_data[i + 3]);
        result.push_back((b[0] << 2) | (b[1] >> 4));
        if (b[2] < 64)
        {
            result.push_back((b[1] << 4) | (b[2] >> 2));
            if (b[3] < 64)
            {
                result.push_back((b[2] << 6) | b[3]);
            }
        }
    }

    return result;
}
}
