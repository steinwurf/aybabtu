// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <system_error>

#include "simd.hpp"

#include "version.hpp"

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
struct base64
{
    /// The size of the encoded data.
    /// @param size size of the data to be encoded
    /// @return the size of the encoded string
    constexpr static std::size_t encode_size(std::size_t size)
    {
        return ((4 * size / 3) + 3) & ~3;
    }

    /// The size of the decoded data.
    /// @param encoded_string the encoded string
    /// @param size the size of the encoded string, must be a multiple of 4
    ///             since the encoded string is padded with '='
    /// @return the size of the decoded data in bytes
    static std::size_t decode_size(const char* encoded_string, std::size_t size)
    {
        assert(size % 4 == 0);
        assert(encoded_string != nullptr);
        // Each Base64 digit represents exactly 6 bits of data. So, three 8-bits
        // bytes of data (3×8 bits = 24 bits) can be
        // represented by four 6-bit Base64 digits (4×6 = 24 bits). This means
        // that the Base64 version of a string or file will be at least 133% the
        // size of its source (a ~33% increase).
        std::size_t result = size / 4 * 3;

        // If the last two characters are == then we have two padding bytes
        if (size >= 1 && encoded_string[size - 1] == '=')
        {
            result--;
            if (size >= 2 && encoded_string[size - 2] == '=')
            {
                result--;
            }
        }
        return result;
    }

    /// The size of the decoded data.
    /// @param string the encoded string
    /// @return the size of the decoded data in bytes
    static std::size_t decode_size(const std::string& string)
    {
        return decode_size(string.c_str(), string.size());
    }

    /// Encode data into a base64 string.
    /// @param data the data to be encoded
    /// @param size the size of the data to be encoded
    /// @param simd the simd instruction set to use, by default auto is used
    ///             which will select the best available instruction set.
    /// @return the encoded string
    static std::string encode(const uint8_t* data, std::size_t size,
                              simd simd = simd::auto_)
    {
        assert(data != nullptr);
        char* out = new char[encode_size(size)];
        std::size_t encoded_size = encode(data, size, out, simd);
        std::string result(out, encoded_size);
        delete[] out;
        return result;
    }

    /// Decode base64 string into data.
    /// @param string the encoded string
    /// @param data the data to be decoded, must be at least as large as the
    ///             result of decode_size(string)
    /// @param error a reference to an error code which will be set if an error
    ///              occurs
    /// @param simd the simd instruction set to use, by default auto is used
    ///             which will select the best available instruction set.
    /// @return the size of the decoded data
    static std::size_t decode(const std::string& string, uint8_t* data,
                              std::error_code& error,
                              simd simd = simd::auto_) noexcept
    {
        assert(data != nullptr);
        assert(!error);
        return decode(string.data(), string.size(), data, error, simd);
    }

    /// Decode base64 string into data.
    /// @param string the encoded string
    /// @param data the data to be decoded
    /// @param simd the simd instruction set to use, by default auto is used
    ///             which will select the best available instruction set.
    /// @return the size of the decoded data
    static std::size_t decode(const std::string& string, uint8_t* data,
                              simd simd = simd::auto_)
    {
        std::error_code error;
        auto result = decode(string, data, error, simd);
        // throw if error
        if (error)
        {
            throw std::system_error(error);
        }
        return result;
    }

    /// Encode a pointer and size to a base64 encoded string
    ///
    /// @param data a pointer to the data
    /// @param size the size of the data in bytes
    /// @param out the output string
    /// @param simd the simd instruction set to use, by default auto is used
    ///             which will select the best available instruction set.
    /// @return the number of bytes written to the data pointer
    static std::size_t encode(const uint8_t* data, std::size_t size, char* out,
                              simd simd = simd::auto_);

    /// Decode a base64 encoded string to a given pointer
    ///
    /// @param string the encoded string
    /// @param size the size of the encoded string
    /// @param out a pointer to the output data
    /// @param error a reference to an error code which will be set if an error
    ///              occurs
    /// @param simd the simd instruction set to use, by default auto is used
    ///             which will select the best available instruction set.
    /// @return the number of bytes written to the data pointer
    static std::size_t decode(const char* string, std::size_t size,
                              uint8_t* out, std::error_code& error,
                              simd simd = simd::auto_) noexcept;
};
}
}
