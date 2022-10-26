// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

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
    static std::size_t encode_size(std::size_t size);

    /// The size of the decoded data.
    /// @param encoded_string the encoded string
    /// @param size the size of the encoded string
    /// @return the size of the decoded data in bytes
    static std::size_t decode_size(const char* encoded_string,
                                   std::size_t size);

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
