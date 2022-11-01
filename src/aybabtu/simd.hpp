// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "version.hpp"

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
/// The SIMD acceleration
enum class simd
{
    /// Automated Acceleration
    auto_,
    /// No Acceleration
    none,
    /// SSSE3 Acceleration
    ssse3,
    /// AVX2 Acceleration
    avx2,
    /// NEON Acceleration
    neon
};
}
}
