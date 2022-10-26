// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64::encode(const uint8_t* data, std::size_t size)
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

std::size_t base64::compute_size(const std::string& encoded_data)
{
    assert(encoded_data.size() % 4 == 0);
    // Each Base64 digit represents exactly 6 bits of data. So, three 8-bits
    // bytes of data (3×8 bits = 24 bits) can be
    // represented by four 6-bit Base64 digits (4×6 = 24 bits). This means that
    // the Base64 version of a string or file will be at least 133% the size of
    // its source (a ~33% increase).
    std::size_t size = encoded_data.size() / 4 * 3;

    // If the last two characters are == then we have two padding bytes
    if (encoded_data[encoded_data.size() - 1] == '=')
    {
        size--;
        if (encoded_data[encoded_data.size() - 2] == '=')
        {
            size--;
        }
    }
    return size;
}

std::size_t base64::decode(uint8_t* data, const std::string& encoded_data)
{
    assert(encoded_data.size() % 4 == 0);

    std::size_t position = 0;
    uint8_t b[4];
    for (std::size_t i = 0U; i < encoded_data.length(); i += 4)
    {
        b[0] = (uint8_t)base64_chars.find(encoded_data[i]);
        b[1] = (uint8_t)base64_chars.find(encoded_data[i + 1]);
        b[2] = (uint8_t)base64_chars.find(encoded_data[i + 2]);
        b[3] = (uint8_t)base64_chars.find(encoded_data[i + 3]);
        data[position++] = (b[0] << 2) | (b[1] >> 4);
        if (b[2] < 64)
        {
            data[position++] = (b[1] << 4) | (b[2] >> 2);
            if (b[3] < 64)
            {
                data[position++] = (b[2] << 6) | b[3];
            }
        }
    }

    return position;
}
}
}
