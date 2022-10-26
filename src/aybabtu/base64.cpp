// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64.hpp"
#include "base64_avx2.hpp"
#include "base64_basic.hpp"

#include <cpuid/cpuinfo.hpp>
#include <platform/config.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
const static cpuid::cpuinfo cpuinfo{};

std::size_t base64::encode_size(std::size_t size)
{
    return ((4 * size / 3) + 3) & ~3;
}

std::size_t base64::decode_size(const char* encoded_string, std::size_t size)
{
    assert(size % 4 == 0);
    // Each Base64 digit represents exactly 6 bits of data. So, three 8-bits
    // bytes of data (3×8 bits = 24 bits) can be
    // represented by four 6-bit Base64 digits (4×6 = 24 bits). This means that
    // the Base64 version of a string or file will be at least 133% the size of
    // its source (a ~33% increase).
    std::size_t result = size / 4 * 3;

    // If the last two characters are == then we have two padding bytes
    if (encoded_string[size - 1] == '=')
    {
        result--;
        if (encoded_string[size - 2] == '=')
        {
            result--;
        }
    }
    return result;
}

std::size_t base64::encode(const uint8_t* data, std::size_t size, char* out)
{
#if defined(PLATFORM_X86)
    if (base64_avx2::is_compiled() && cpuinfo.has_avx2())
    {
        return base64_avx2::encode(data, size, out);
    }
    // if (base64_sse2::is_compiled() && cpuinfo.has_sse2())
    // {
    //     // return base64_sse2::encode(data, size, out);
    // }
#elif defined(PLATFORM_ARM)
    // if (base64_neon::is_compiled() && cpuinfo.has_neon())
    // {
    //     return base64_neon::encode(data, size, out);
    // }
#endif
    return base64_basic::encode(data, size, out);
}

std::size_t base64::decode(const char* encoded_string, std::size_t size,
                           uint8_t* data)
{
#if defined(PLATFORM_X86)
    // if (base64_avx2::is_compiled() && cpuinfo.has_avx2())
    // {
    //     return base64_avx2::decode(encoded_string, size, data);
    // }

    // if (base64_sse2::is_compiled() && cpuinfo.has_sse2())
    // {
    //     return base64_sse2::decode(encoded_string, size, data);
    // }
#elif defined(PLATFORM_ARM)
    // if (base64_neon::is_compiled() && cpuinfo.has_neon())
    // {
    //     return base64_neon::decode(encoded_string, size, data);
    // }
#endif
    return base64_basic::decode(encoded_string, size, data);
}
}
}
