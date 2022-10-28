// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64_basic.hpp"
#include "base64_decode.hpp"
#include "base64_encode.hpp"

#include <cassert>
#include <cstdint>

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
namespace detail
{

static inline void noop(const uint8_t**, std::size_t&, uint8_t**, size_t&)
{
}

std::size_t base64_basic::encode(const uint8_t* src, std::size_t size,
                                 uint8_t* out)
{

    return base64_encode(&noop, src, size, out);
}

std::size_t base64_basic::decode(const uint8_t* src, std::size_t size,
                                 uint8_t* out)

{
    return base64_decode(&noop, src, size, out);
}
}
}
}
