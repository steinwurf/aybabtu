// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <aybabtu/base64.hpp>

#include <iostream>

int main()
{
    std::vector<uint8_t> buffer{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
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
