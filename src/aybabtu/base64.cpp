// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64.hpp"
#include "detail/base64_avx2.hpp"
#include "detail/base64_basic.hpp"
#include "detail/base64_neon.hpp"
#include "detail/base64_ssse3.hpp"

#include "version.hpp"

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

std::size_t base64::encode(const uint8_t* data, std::size_t size, char* out,
                           simd simd)
{
#if defined(PLATFORM_X86)
    if ((simd == simd::auto_ && detail::base64_avx2::is_compiled() &&
         cpuinfo.has_avx2()) ||
        simd == simd::avx2)
    {
        return detail::base64_avx2::encode(data, size, (uint8_t*)out);
    }
    if ((simd == simd::auto_ && detail::base64_ssse3::is_compiled() &&
         cpuinfo.has_ssse3()) ||
        simd == simd::ssse3)
    {
        return detail::base64_ssse3::encode(data, size, (uint8_t*)out);
    }
#elif defined(PLATFORM_ARM)
    if ((simd == simd::auto_ && detail::base64_neon::is_compiled() &&
         cpuinfo.has_neon()) ||
        simd == simd::neon)
    {
        return detail::base64_neon::encode(data, size, (uint8_t*)out);
    }
#endif
    return detail::base64_basic::encode(data, size, (uint8_t*)out);
}

std::size_t base64::decode(const char* string, std::size_t size, uint8_t* out,
                           simd simd)
{
#if defined(PLATFORM_X86)
    if ((simd == simd::auto_ && detail::base64_avx2::is_compiled() &&
         cpuinfo.has_avx2()) ||
        simd == simd::avx2)
    {
        return detail::base64_avx2::decode((const uint8_t*)string, size, out);
    }

    if ((simd == simd::auto_ && detail::base64_ssse3::is_compiled() &&
         cpuinfo.has_ssse3()) ||
        simd == simd::ssse3)
    {
        return detail::base64_ssse3::decode((const uint8_t*)string, size, out);
    }
#elif defined(PLATFORM_ARM)
    if ((simd == simd::auto_ && detail::base64_neon::is_compiled() &&
         cpuinfo.has_neon()) ||
        simd == simd::neon)
    {
        return detail::base64_neon::decode((const uint8_t*)string, size, out);
    }
#endif
    return detail::base64_basic::decode((const uint8_t*)string, size, out);
}
}
}
