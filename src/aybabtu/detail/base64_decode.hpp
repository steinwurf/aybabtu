// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "tables.hpp"

#include <cassert>
#include <cstdint>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
namespace detail
{
template <class Func>
static inline std::size_t base64_decode(Func func, const uint8_t* src,
                                        std::size_t size, uint8_t* out)
{
    std::size_t written = 0;
    std::size_t remaining = size;

    // Turn four 6-bit numbers into three bytes:
    // out[0] = 11111122
    // out[1] = 22223333
    // out[2] = 33444444

    while (true)
    {
        func(&src, remaining, &out, written);
        if (remaining-- == 0)
        {
            return written;
        }

        uint8_t q = tables::decode[*src++];
        if (q >= 254)
        {
            // Treat character '=' as invalid for byte 0:
            assert(false);
            return 0;
        }
        std::size_t carry = q << 2;

        if (remaining-- == 0)
        {
            return written;
        }
        q = tables::decode[*src++];
        if (q >= 254)
        {
            // Treat character '=' as invalid for byte 1:
            assert(false);
            return 0;
        }
        *out++ = carry | (q >> 4);
        carry = q << 4;
        written++;

        if (remaining-- == 0)
        {
            return written;
        }
        q = tables::decode[*src++];
        if (q >= 254)
        {
            // When q == 254, the input char is '='.
            if (q == 254)
            {
                // Check if next byte is also '=':
                if (remaining == 1)
                {
                    remaining--;
                    q = tables::decode[*src++];
                    assert(q == 254);
                    return written;
                }
            }
            // If we get here, there was an error:
            assert(false);
            return 0;
        }
        *out++ = carry | (q >> 2);
        carry = q << 6;
        written++;

        if (remaining-- == 0)
        {
            return written;
        }
        q = tables::decode[*src++];
        // When q == 254, the input char is '='.
        if (q == 254)
        {
            return written;
        }
        if (q == 255)
        {
            assert(false);
            return 0;
        }

        *out++ = carry | q;
        written++;
    }
}
}
}
}
