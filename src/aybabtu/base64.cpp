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
                           uint8_t* out)
{
#if defined(PLATFORM_X86)
    if (base64_avx2::is_compiled() && cpuinfo.has_avx2())
    {
        return base64_avx2::decode(encoded_string, size, out);
    }

    // if (base64_sse2::is_compiled() && cpuinfo.has_sse2())
    // {
    //     return base64_sse2::decode(encoded_string, size, out);
    // }
#elif defined(PLATFORM_ARM)
    // if (base64_neon::is_compiled() && cpuinfo.has_neon())
    // {
    //     return base64_neon::decode(encoded_string, size, out);
    // }
#endif
    return base64_basic::decode(encoded_string, size, out);
}
}
}
