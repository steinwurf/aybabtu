// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64_ssse3.hpp"
#include "../version.hpp"
#include "base64_decode.hpp"
#include "base64_encode.hpp"

#include <platform/config.hpp>

#include <cassert>
#include <cstdint>

// Include x86 intrinsics for GCC-compatible compilers on x86/x86_64
#if defined(PLATFORM_GCC_COMPATIBLE_X86)
#include <x86intrin.h>
#elif defined(PLATFORM_MSVC_X86)
#include <immintrin.h>
#endif

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
namespace detail
{
#ifdef PLATFORM_SSSE3

// This code borrows from code from Alfred Klomp's library
// https://github.com/aklomp/base64 (published under BSD)
// The code has been modified to fit the aybabtu library.
static inline __m128i enc_reshuffle(__m128i in)
{
    // Input, bytes MSB to LSB:
    // 0 0 0 0 l k j i h g f e d c b a

    in = _mm_shuffle_epi8(
        in, _mm_set_epi8(10, 11, 9, 10, 7, 8, 6, 7, 4, 5, 3, 4, 1, 2, 0, 1));
    // in, bytes MSB to LSB:
    // k l j k
    // h i g h
    // e f d e
    // b c a b

    const __m128i t0 = _mm_and_si128(in, _mm_set1_epi32(0x0FC0FC00));
    // bits, upper case are most significant bits, lower case are least
    // significant bits 0000kkkk LL000000 JJJJJJ00 00000000 0000hhhh II000000
    // GGGGGG00 00000000 0000eeee FF000000 DDDDDD00 00000000 0000bbbb CC000000
    // AAAAAA00 00000000

    const __m128i t1 = _mm_mulhi_epu16(t0, _mm_set1_epi32(0x04000040));
    // 00000000 00kkkkLL 00000000 00JJJJJJ
    // 00000000 00hhhhII 00000000 00GGGGGG
    // 00000000 00eeeeFF 00000000 00DDDDDD
    // 00000000 00bbbbCC 00000000 00AAAAAA

    const __m128i t2 = _mm_and_si128(in, _mm_set1_epi32(0x003F03F0));
    // 00000000 00llllll 000000jj KKKK0000
    // 00000000 00iiiiii 000000gg HHHH0000
    // 00000000 00ffffff 000000dd EEEE0000
    // 00000000 00cccccc 000000aa BBBB0000

    const __m128i t3 = _mm_mullo_epi16(t2, _mm_set1_epi32(0x01000010));
    // 00llllll 00000000 00jjKKKK 00000000
    // 00iiiiii 00000000 00ggHHHH 00000000
    // 00ffffff 00000000 00ddEEEE 00000000
    // 00cccccc 00000000 00aaBBBB 00000000

    return _mm_or_si128(t1, t3);
    // 00llllll 00kkkkLL 00jjKKKK 00JJJJJJ
    // 00iiiiii 00hhhhII 00ggHHHH 00GGGGGG
    // 00ffffff 00eeeeFF 00ddEEEE 00DDDDDD
    // 00cccccc 00bbbbCC 00aaBBBB 00AAAAAA
}
static inline __m128i enc_translate(const __m128i in)
{
    // A lookup table containing the absolute offsets for all ranges:
    const __m128i lut = _mm_setr_epi8(65, 71, -4, -4, -4, -4, -4, -4, -4, -4,
                                      -4, -4, -19, -16, 0, 0);

    // Translate values 0..63 to the Base64 alphabet. There are five sets:
    // #  From      To         Abs    Index  Characters
    // 0  [0..25]   [65..90]   +65        0  ABCDEFGHIJKLMNOPQRSTUVWXYZ
    // 1  [26..51]  [97..122]  +71        1  abcdefghijklmnopqrstuvwxyz
    // 2  [52..61]  [48..57]    -4  [2..11]  0123456789
    // 3  [62]      [43]       -19       12  +
    // 4  [63]      [47]       -16       13  /

    // Create LUT indices from the input. The index for range #0 is right,
    // others are 1 less than expected:
    __m128i indices = _mm_subs_epu8(in, _mm_set1_epi8(51));

    // mask is 0xFF (-1) for range #[1..4] and 0x00 for range #0:
    __m128i mask = _mm_cmpgt_epi8(in, _mm_set1_epi8(25));

    // Subtract -1, so add 1 to indices for range #[1..4]. All indices are
    // now correct:
    indices = _mm_sub_epi8(indices, mask);

    // Add offsets to input values:
    return _mm_add_epi8(in, _mm_shuffle_epi8(lut, indices));
}
static inline void encode_loop_ssse3(const uint8_t** src,
                                     std::size_t& remaining, uint8_t** out,
                                     std::size_t& written)
{
    if (remaining < 16)
    {
        return;
    }

    // Process blocks of 12 bytes at a time. Because blocks are loaded 16
    // bytes at a time, ensure that there will be at least 4 remaining
    // bytes after the last round, so that the final read will not pass
    // beyond the bounds of the input buffer:
    size_t rounds = (remaining - 4) / 12;

    while (rounds > 0)
    {
        // Load input:
        __m128i str = _mm_loadu_si128((__m128i*)*src);

        // Reshuffle:
        str = enc_reshuffle(str);

        // Translate reshuffled bytes to the Base64 alphabet:
        str = enc_translate(str);

        // Store:
        _mm_storeu_si128((__m128i*)*out, str);

        *src += 12;
        *out += 16;
        remaining -= 12; // 12 bytes consumed per round
        written += 16;   // 16 bytes produced per round

        rounds--;
    }
}

static inline __m128i dec_reshuffle(const __m128i in)
{
    // in, bits, upper case are most significant bits, lower case
    // are least significant bits
    // 00llllll 00kkkkLL 00jjKKKK 00JJJJJJ
    // 00iiiiii 00hhhhII 00ggHHHH 00GGGGGG
    // 00ffffff 00eeeeFF 00ddEEEE 00DDDDDD
    // 00cccccc 00bbbbCC 00aaBBBB 00AAAAAA

    const __m128i merge_ab_and_bc =
        _mm_maddubs_epi16(in, _mm_set1_epi32(0x01400140));
    // 0000kkkk LLllllll 0000JJJJ JJjjKKKK
    // 0000hhhh IIiiiiii 0000GGGG GGggHHHH
    // 0000eeee FFffffff 0000DDDD DDddEEEE
    // 0000bbbb CCcccccc 0000AAAA AAaaBBBB

    const __m128i out =
        _mm_madd_epi16(merge_ab_and_bc, _mm_set1_epi32(0x00011000));
    // 00000000 JJJJJJjj KKKKkkkk LLllllll
    // 00000000 GGGGGGgg HHHHhhhh IIiiiiii
    // 00000000 DDDDDDdd EEEEeeee FFffffff
    // 00000000 AAAAAAaa BBBBbbbb CCcccccc

    // Pack bytes together:
    return _mm_shuffle_epi8(out, _mm_setr_epi8(2, 1, 0, 6, 5, 4, 10, 9, 8, 14,
                                               13, 12, -1, -1, -1, -1));
    // 00000000 00000000 00000000 00000000
    // LLllllll KKKKkkkk JJJJJJjj IIiiiiii
    // HHHHhhhh GGGGGGgg FFffffff EEEEeeee
    // DDDDDDdd CCcccccc BBBBbbbb AAAAAAaa
}
static inline int dec_loop_ssse3_inner(const uint8_t** s, uint8_t** o,
                                       size_t& rounds)
{
    const __m128i lut_lo =
        _mm_setr_epi8(0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                      0x11, 0x13, 0x1A, 0x1B, 0x1B, 0x1B, 0x1A);

    const __m128i lut_hi =
        _mm_setr_epi8(0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08, 0x10,
                      0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10);

    const __m128i lut_roll =
        _mm_setr_epi8(0, 16, 19, 4, -65, -65, -71, -71, 0, 0, 0, 0, 0, 0, 0, 0);

    const __m128i mask_2F = _mm_set1_epi8(0x2F);

    // Load input:
    __m128i str = _mm_loadu_si128((__m128i*)*s);

    // Table lookups:
    const __m128i hi_nibbles = _mm_and_si128(_mm_srli_epi32(str, 4), mask_2F);
    const __m128i lo_nibbles = _mm_and_si128(str, mask_2F);
    const __m128i hi = _mm_shuffle_epi8(lut_hi, hi_nibbles);
    const __m128i lo = _mm_shuffle_epi8(lut_lo, lo_nibbles);

    // Check for invalid input: if any "and" values from lo and hi are not
    // zero, fall back on bytewise code to do error checking and reporting:
    if (_mm_movemask_epi8(
            _mm_cmpgt_epi8(_mm_and_si128(lo, hi), _mm_setzero_si128())) != 0)
    {
        return 0;
    }

    const __m128i eq_2F = _mm_cmpeq_epi8(str, mask_2F);
    const __m128i roll =
        _mm_shuffle_epi8(lut_roll, _mm_add_epi8(eq_2F, hi_nibbles));

    // Now simply add the delta values to the input:
    str = _mm_add_epi8(str, roll);

    // Reshuffle the input to packed 12-byte output format:
    str = dec_reshuffle(str);

    // Store the output:
    _mm_storeu_si128((__m128i*)*o, str);

    *s += 16;
    *o += 12;
    rounds -= 1;

    return 1;
}

static inline void decode_loop_ssse3(const uint8_t** src,
                                     std::size_t& remaining, uint8_t** out,
                                     std::size_t& written)
{
    if (remaining < 24)
    {
        return;
    }

    // Process blocks of 16 bytes per round. Because 4 extra zero bytes are
    // written after the output, ensure that there will be at least 8 bytes
    // of input data left to cover the gap. (6 data bytes and up to two
    // end-of-string markers.)
    size_t rounds = (remaining - 8) / 16;

    const __m128i lut_lo =
        _mm_setr_epi8(0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                      0x11, 0x13, 0x1A, 0x1B, 0x1B, 0x1B, 0x1A);

    const __m128i lut_hi =
        _mm_setr_epi8(0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08, 0x10,
                      0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10);

    const __m128i lut_roll =
        _mm_setr_epi8(0, 16, 19, 4, -65, -65, -71, -71, 0, 0, 0, 0, 0, 0, 0, 0);

    const __m128i mask_2F = _mm_set1_epi8(0x2F);

    while (rounds > 0)
    {
        // Load input:
        __m128i str = _mm_loadu_si128((__m128i*)*src);

        // Table lookups:
        const __m128i hi_nibbles =
            _mm_and_si128(_mm_srli_epi32(str, 4), mask_2F);
        const __m128i lo_nibbles = _mm_and_si128(str, mask_2F);
        const __m128i hi = _mm_shuffle_epi8(lut_hi, hi_nibbles);
        const __m128i lo = _mm_shuffle_epi8(lut_lo, lo_nibbles);

        // Check for invalid input: if any "and" values from lo and hi are not
        // zero, fall back on bytewise code to do error checking and reporting:
        if (_mm_movemask_epi8(_mm_cmpgt_epi8(_mm_and_si128(lo, hi),
                                             _mm_setzero_si128())) != 0)
        {
            continue;
        }

        const __m128i eq_2F = _mm_cmpeq_epi8(str, mask_2F);
        const __m128i roll =
            _mm_shuffle_epi8(lut_roll, _mm_add_epi8(eq_2F, hi_nibbles));

        // Now simply add the delta values to the input:
        str = _mm_add_epi8(str, roll);

        // Reshuffle the input to packed 12-byte output format:
        str = dec_reshuffle(str);

        // Store the output:
        _mm_storeu_si128((__m128i*)*out, str);

        *src += 16;
        *out += 12;
        remaining -= 16; // 16 bytes consumed per round
        written += 12;   // 12 bytes produced per round
        rounds -= 1;
    }
}

std::size_t base64_ssse3::encode(const uint8_t* src, std::size_t size,
                                 uint8_t* out)
{
    return base64_encode(&encode_loop_ssse3, src, size, out);
}

std::size_t base64_ssse3::decode(const uint8_t* src, std::size_t size,
                                 uint8_t* out, std::error_code& error)
{
    return base64_decode(&decode_loop_ssse3, src, size, out, error);
}

bool base64_ssse3::is_compiled()
{
    return true;
}
#else
std::size_t base64_ssse3::encode(const uint8_t*, std::size_t, uint8_t*)
{
    assert(0 && "Target platform or compiler does not support this "
                "implementation");
    return 0;
}

std::size_t base64_ssse3::decode(const uint8_t*, std::size_t, uint8_t*,
                                 std::error_code&)
{
    assert(0 && "Target platform or compiler does not support this "
                "implementation");
    return 0;
}

bool base64_ssse3::is_compiled()
{
    return false;
}
#endif
}
}
}
