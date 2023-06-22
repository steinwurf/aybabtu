// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "../version.hpp"
#include "tables.hpp"

#include <cstdint>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
namespace detail
{

template <class Func, int Chunk = 0>
static inline std::size_t base64_encode(Func func, const uint8_t* src,
                                        std::size_t size, uint8_t* out)
{
    std::size_t written = 0;
    std::size_t remaining = size;

    while (true)
    {
        func(&src, remaining, &out, written);
        if (remaining == 0)
        {
            return written;
        }
        *out++ = tables::encode[*src >> 2];
        written += 1;
        remaining -= 1;

        uint8_t carry = (*src++ << 4) & 0x30;
        if (remaining == 0)
        {
            *out++ = tables::encode[carry];
            *out++ = '=';
            *out++ = '=';
            written += 3;
            return written;
        }
        *out++ = tables::encode[carry | (*src >> 4)];
        written += 1;
        remaining -= 1;

        carry = (*src++ << 2) & 0x3C;
        if (remaining == 0)
        {
            *out++ = tables::encode[carry];
            *out++ = '=';
            written += 2;
            return written;
        }
        *out++ = tables::encode[carry | (*src >> 6)];
        *out++ = tables::encode[*src++ & 0x3F];
        written += 2;
        remaining -= 1;
    }
}
}
}
}
