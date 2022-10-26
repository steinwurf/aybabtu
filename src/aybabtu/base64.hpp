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
    /// Create a base64 encoded string from a pointer and size.
    ///
    /// @param data a pointer to the data
    /// @param size the size of the data in bytes
    static std::string encode(const uint8_t* data, std::size_t size);

    /// Compute the size of the decoded data.
    /// Each Base64 digit represents exactly 6 bits of data. So, three 8-bits
    /// bytes of data (3×8 bits = 24 bits) can be
    /// represented by four 6-bit Base64 digits (4×6 = 24 bits). This means that
    /// the Base64 version of a string or file will be at least 133% the size of
    /// its source (a ~33% increase).
    /// @param encoded_data the encoded data
    /// @return the size of the decoded data in bytes
    static std::size_t compute_size(const std::string& encoded_data);

    /// Decode a base64 encoded string to a given pointer
    ///
    /// @param data a pointer to the data
    /// @param encoded_data A string containing the encoded data.
    /// @return the number of bytes written to the data pointer
    static std::size_t decode(uint8_t* data, const std::string& encoded_data);
};
}
}
