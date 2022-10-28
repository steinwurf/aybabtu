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
enum class simd
{
    auto_,
    none,
    ssse3,
    avx2,
    neon
};
}
}
