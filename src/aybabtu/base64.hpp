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
struct base64
{

    /// Creates a base64 encoded string from a vector.
    ///
    /// @param buffer A vector containing the data to encode.
    static std::string encode(const std::vector<uint8_t>& buffer);

    /// Creates a base64 encoded string from a buffer of the specified size.
    ///
    /// @param data a pointer to the data
    /// @param size the size of the data in bytes
    static std::string encode(const uint8_t* data, uint32_t size);

    /// Decodes a base64 encoded string to a vector.
    ///
    /// @param encoded_data A string containing the encoded data.
    static std::vector<uint8_t> decode(const std::string& encoded_data);
};
}
