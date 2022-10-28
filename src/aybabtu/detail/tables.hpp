// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "../version.hpp"

#include <cstdint>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
namespace detail
{
struct tables
{
    static const uint8_t encode[];
    static const uint8_t decode[];
};
}
}
}
