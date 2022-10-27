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
struct base64_avx2
{
    static std::size_t encode(const uint8_t* data, std::size_t size, char* out);

    static std::size_t decode(const char* encoded_string, std::size_t size,
                              uint8_t* out);

    /// @return whether this cpu acceralation is compiled or not
    static bool is_compiled();
};
}
}
