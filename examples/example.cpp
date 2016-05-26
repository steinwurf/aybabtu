// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <aybabtu/base64.hpp>

#include <iostream>

int main()
{
    std::vector<uint8_t> data { 1 ,2 ,3, 4, 5, 6, 7, 8, 9, 10 };
    auto encoded = aybabtu::base64::encode(data);
    std::cout << "base64 encoded string: " << encoded << std::endl;
    std::cout << "Trying to decode..." << std::endl;
    auto result = aybabtu::base64::decode(encoded);
    if (result == data)
    {
        std::cout << "Decoded correctly." << std::endl;
    }
    else
    {
        std::cout << "Decoding failed." << std::endl;
    }

    return 0;
}
