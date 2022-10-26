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
struct base64_basic
{
    /// Create a base64 encoded string from a pointer and size.
    ///
    /// @param data a pointer to the data
    /// @param size the size of the data in bytes
    static std::string encode(const uint8_t* data, std::size_t size);

    /// Decode a base64 encoded string to a given pointer
    ///
    /// @param data a pointer to the data
    /// @param encoded_data A string containing the encoded data.
    /// @return the number of bytes written to the data pointer
    static std::size_t decode(uint8_t* data, const std::string& encoded_data);
};
}
}
