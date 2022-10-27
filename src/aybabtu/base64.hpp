// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
struct base64
{
    /// The size of the encoded data.
    /// @param size size of the data to be encoded
    /// @return the size of the encoded string
    constexpr static std::size_t encode_size(std::size_t size)
    {
        return ((4 * size / 3) + 3) & ~3;
    }

    /// The size of the decoded data.
    /// @param encoded_string the encoded string
    /// @param size the size of the encoded string
    /// @return the size of the decoded data in bytes
    constexpr static std::size_t decode_size(const char* encoded_string,
                                             std::size_t size)
    {
        assert(size % 4 == 0);
        // Each Base64 digit represents exactly 6 bits of data. So, three 8-bits
        // bytes of data (3×8 bits = 24 bits) can be
        // represented by four 6-bit Base64 digits (4×6 = 24 bits). This means
        // that the Base64 version of a string or file will be at least 133% the
        // size of its source (a ~33% increase).
        std::size_t result = size / 4 * 3;

        // If the last two characters are == then we have two padding bytes
        if (size >= 1 && encoded_string[size - 1] == '=')
        {
            result--;
            if (size >= 2 && encoded_string[size - 2] == '=')
            {
                result--;
            }
        }
        return result;
    }

    /// Encode a pointer and size to a base64 encoded string
    ///
    /// @param data a pointer to the data
    /// @param size the size of the data in bytes
    /// @param out the output string
    /// @return the number of bytes written to the data pointer
    static std::size_t encode(const uint8_t* data, std::size_t size, char* out);

    /// Decode a base64 encoded string to a given pointer
    ///
    /// @param encoded_string the encoded string
    /// @param size the size of the encoded string
    /// @param data a pointer to the data
    /// @return the number of bytes written to the data pointer
    static std::size_t decode(const char* encoded_string, std::size_t size,
                              uint8_t* data);
};
}
}
