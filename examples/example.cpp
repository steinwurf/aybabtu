// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <aybabtu/base64.hpp>

#include <iostream>

int main()
{
    std::vector<uint8_t> buffer{0x02, 0x59, 0x58, 0xa2, 0xea, 0xc1,
                                0x6a, 0xc7, 0x80, 0xad, 0xe0, 0x5e,
                                0x96, 0x89, 0xe0, 0x4e, 0x85, 0x2c};
    auto encoded = aybabtu::base64::encode(buffer.data(), buffer.size());
    std::cout << "base64 encoded string: " << encoded << std::endl;
    std::cout << "Trying to decode..." << std::endl;
    std::vector<uint8_t> decoded(aybabtu::base64::compute_size(encoded));
    aybabtu::base64::decode(decoded.data(), encoded);

    if (decoded == buffer)
    {
        std::cout << "Decoded correctly." << std::endl;
    }
    else
    {
        std::cout << "Decoding failed." << std::endl;
    }

    return 0;
}
