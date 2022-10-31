// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "../version.hpp"

#include <cstdint>
#include <system_error>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
namespace detail
{
struct base64_ssse3
{
    static std::size_t encode(const uint8_t* src, std::size_t size,
                              uint8_t* out);

    static std::size_t decode(const uint8_t* src, std::size_t size,
                              uint8_t* out, std::error_code& error);

    /// @return whether this cpu acceralation is compiled or not
    static bool is_compiled();
};
}
}
}
