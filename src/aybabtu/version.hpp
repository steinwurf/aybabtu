// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <string>

namespace aybabtu
{
/// Here we define the STEINWURF_AYBABTU_VERSION this should be updated on each
/// release
#define STEINWURF_AYBABTU_VERSION v15_0_0

inline namespace STEINWURF_AYBABTU_VERSION
{
/// @return The version of the library as string
std::string version();
}
}
