// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64.hpp"
#include "base64_basic.hpp"

#include <platform/config.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
const cpuid::cpuinfo base64::cpuinfo{};
std::string base64::encode(const uint8_t* data, std::size_t size)

{
    return base64_basic::encode(data, size);
    // #if defined(PLATFORM_X86)
    //     if (cpuinfo.has_avx2())
    //     {
    //         return base64_basic::encode(data, size);
    //         // return base64_avx2::encode(data, size);
    //     }
    //     else if (cpuinfo.has_sse2())
    //     {
    //         return base64_basic::encode(data, size);
    //         // return base64_sse2::encode(data, size);
    //     }
    //     else
    //     {
    //         return base64_basic::encode(data, size);
    //     }
    // #elif defined(PLATFORM_NEON)
    //     if (cpuinfo.has_neon())
    //     {
    //         return base64_basic::encode(data, size);
    //         // return base64_neon::encode(data, size);
    //     }
    //     else
    //     {
    //         return base64_basic::encode(data, size);
    //     }
    // #else
    //     return base64_basic::encode(data, size);
    // #endif
}

std::size_t base64::compute_size(const std::string& encoded_data)
{
    assert(encoded_data.size() % 4 == 0);
    // Each Base64 digit represents exactly 6 bits of data. So, three 8-bits
    // bytes of data (3×8 bits = 24 bits) can be
    // represented by four 6-bit Base64 digits (4×6 = 24 bits). This means that
    // the Base64 version of a string or file will be at least 133% the size of
    // its source (a ~33% increase).
    std::size_t size = encoded_data.size() / 4 * 3;

    // If the last two characters are == then we have two padding bytes
    if (encoded_data[encoded_data.size() - 1] == '=')
    {
        size--;
        if (encoded_data[encoded_data.size() - 2] == '=')
        {
            size--;
        }
    }
    return size;
}

std::size_t base64::decode(uint8_t* data, const std::string& encoded_data)
{
    return base64_basic::decode(data, encoded_data);
    // #if defined(PLATFORM_X86)
    //     if (cpuinfo.has_avx2())
    //     {
    //         return base64_basic::decode(data, encoded_data);
    //         // return base64_avx2::decode(data, encoded_data);
    //     }
    //     else if (cpuinfo.has_sse2())
    //     {
    //         return base64_basic::decode(data, encoded_data);
    //         // return base64_sse2::decode(data, encoded_data);
    //     }
    //     else
    //     {
    //         return base64_basic::decode(data, encoded_data);
    //     }
    // #elif defined(PLATFORM_NEON)
    //     if (cpuinfo.has_neon())
    //     {
    //         return base64_basic::decode(data, encoded_data);
    //         // return base64_neon::decode(data, encoded_data);
    //     }
    //     else
    //     {
    //         return base64_basic::decode(data, encoded_data);
    //     }
    // #else
    //     return base64_basic::decode(data, encoded_data);
    // #endif
}
}
}
