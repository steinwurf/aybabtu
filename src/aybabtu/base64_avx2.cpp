// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64_avx2.hpp"

#include <platform/config.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

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
#ifdef PLATFORM_AVX2

// This code borrows from code from Alfred Klomp's library
// https://github.com/aklomp/base64 (published under BSD)

const uint8_t base64_table_enc_6bit[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// clang-format off
const uint8_t base64_table_dec_8bit[] =
{
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		//   0..15
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		//  16..31
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,		//  32..47
	 52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 254, 255, 255,		//  48..63
	255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,		//  64..79
	 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,		//  80..95
	255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,		//  96..111
	 41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255,		// 112..127
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,		// 128..143
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};
// clang-format on

static inline __m256i enc_translate(const __m256i in)
{
    // A lookup table containing the absolute offsets for all ranges:
    const __m256i lut = _mm256_setr_epi8(
        65, 71, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -19, -16, 0, 0, 65, 71,
        -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -19, -16, 0, 0);

    // Translate values 0..63 to the Base64 alphabet. There are five sets:
    // #  From      To         Abs    Index  Characters
    // 0  [0..25]   [65..90]   +65        0  ABCDEFGHIJKLMNOPQRSTUVWXYZ
    // 1  [26..51]  [97..122]  +71        1  abcdefghijklmnopqrstuvwxyz
    // 2  [52..61]  [48..57]    -4  [2..11]  0123456789
    // 3  [62]      [43]       -19       12  +
    // 4  [63]      [47]       -16       13  /

    // Create LUT indices from the input. The index for range #0 is right,
    // others are 1 less than expected:
    __m256i indices = _mm256_subs_epu8(in, _mm256_set1_epi8(51));

    // mask is 0xFF (-1) for range #[1..4] and 0x00 for range #0:
    const __m256i mask = _mm256_cmpgt_epi8(in, _mm256_set1_epi8(25));

    // Subtract -1, so add 1 to indices for range #[1..4]. All indices are
    // now correct:
    indices = _mm256_sub_epi8(indices, mask);

    // Add offsets to input values:
    return _mm256_add_epi8(in, _mm256_shuffle_epi8(lut, indices));
}

static inline __m256i enc_reshuffle(const __m256i input)
{
    // Input, bytes MSB to LSB:
    // 0 0 0 0 x w v u t s r q p o n m
    // l k j i h g f e d c b a 0 0 0 0
    const __m256i in = _mm256_shuffle_epi8(
        input,
        _mm256_set_epi8(10, 11, 9, 10, 7, 8, 6, 7, 4, 5, 3, 4, 1, 2, 0, 1, 14,
                        15, 13, 14, 11, 12, 10, 11, 8, 9, 7, 8, 5, 6, 4, 5));
    // in, bytes MSB to LSB:
    // w x v w
    // t u s t
    // q r p q
    // n o m n
    // k l j k
    // h i g h
    // e f d e
    // b c a b

    const __m256i t0 = _mm256_and_si256(in, _mm256_set1_epi32(0x0FC0FC00));
    // bits, upper case are most significant bits, lower case are least
    // significant bits.
    // 0000wwww XX000000 VVVVVV00 00000000
    // 0000tttt UU000000 SSSSSS00 00000000
    // 0000qqqq RR000000 PPPPPP00 00000000
    // 0000nnnn OO000000 MMMMMM00 00000000
    // 0000kkkk LL000000 JJJJJJ00 00000000
    // 0000hhhh II000000 GGGGGG00 00000000
    // 0000eeee FF000000 DDDDDD00 00000000
    // 0000bbbb CC000000 AAAAAA00 00000000

    const __m256i t1 = _mm256_mulhi_epu16(t0, _mm256_set1_epi32(0x04000040));
    // 00000000 00wwwwXX 00000000 00VVVVVV
    // 00000000 00ttttUU 00000000 00SSSSSS
    // 00000000 00qqqqRR 00000000 00PPPPPP
    // 00000000 00nnnnOO 00000000 00MMMMMM
    // 00000000 00kkkkLL 00000000 00JJJJJJ
    // 00000000 00hhhhII 00000000 00GGGGGG
    // 00000000 00eeeeFF 00000000 00DDDDDD
    // 00000000 00bbbbCC 00000000 00AAAAAA

    const __m256i t2 = _mm256_and_si256(in, _mm256_set1_epi32(0x003F03F0));
    // 00000000 00xxxxxx 000000vv WWWW0000
    // 00000000 00uuuuuu 000000ss TTTT0000
    // 00000000 00rrrrrr 000000pp QQQQ0000
    // 00000000 00oooooo 000000mm NNNN0000
    // 00000000 00llllll 000000jj KKKK0000
    // 00000000 00iiiiii 000000gg HHHH0000
    // 00000000 00ffffff 000000dd EEEE0000
    // 00000000 00cccccc 000000aa BBBB0000

    const __m256i t3 = _mm256_mullo_epi16(t2, _mm256_set1_epi32(0x01000010));
    // 00xxxxxx 00000000 00vvWWWW 00000000
    // 00uuuuuu 00000000 00ssTTTT 00000000
    // 00rrrrrr 00000000 00ppQQQQ 00000000
    // 00oooooo 00000000 00mmNNNN 00000000
    // 00llllll 00000000 00jjKKKK 00000000
    // 00iiiiii 00000000 00ggHHHH 00000000
    // 00ffffff 00000000 00ddEEEE 00000000
    // 00cccccc 00000000 00aaBBBB 00000000

    return _mm256_or_si256(t1, t3);
    // 00xxxxxx 00wwwwXX 00vvWWWW 00VVVVVV
    // 00uuuuuu 00ttttUU 00ssTTTT 00SSSSSS
    // 00rrrrrr 00qqqqRR 00ppQQQQ 00PPPPPP
    // 00oooooo 00nnnnOO 00mmNNNN 00MMMMMM
    // 00llllll 00kkkkLL 00jjKKKK 00JJJJJJ
    // 00iiiiii 00hhhhII 00ggHHHH 00GGGGGG
    // 00ffffff 00eeeeFF 00ddEEEE 00DDDDDD
    // 00cccccc 00bbbbCC 00aaBBBB 00AAAAAA
}

static inline void encode_loop_avx2(const uint8_t** src, std::size_t& remaining,
                                    char** out, size_t& written)
{
    if (remaining < 32)
    {
        return;
    }

    // Process blocks of 24 bytes at a time. Because blocks are loaded 32
    // bytes at a time an offset of -4, ensure that there will be at least
    // 4 remaining bytes after the last round, so that the final read will
    // not pass beyond the bounds of the input buffer:
    size_t rounds = (remaining - 4) / 24;

    remaining -= rounds * 24; // 24 bytes consumed per round
    written += rounds * 32;   // 32 bytes produced per round

    // First load is done at s - 0 to not get a segfault:
    __m256i avx_src = _mm256_loadu_si256((__m256i*)*src);

    // Shift by 4 bytes, as required by enc_reshuffle:
    avx_src = _mm256_permutevar8x32_epi32(
        avx_src, _mm256_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6));

    // Reshuffle, translate, store:
    avx_src = enc_reshuffle(avx_src);
    avx_src = enc_translate(avx_src);
    _mm256_storeu_si256((__m256i*)*out, avx_src);

    // Subsequent loads will be done at s - 4, set pointer for next round:
    *src += 20;
    *out += 32;

    rounds--;

    while (rounds > 0)
    {
        // Load input:
        avx_src = _mm256_loadu_si256((__m256i*)*src);

        // Reshuffle, translate, store:
        avx_src = enc_reshuffle(avx_src);
        avx_src = enc_translate(avx_src);
        _mm256_storeu_si256((__m256i*)*out, avx_src);

        *src += 24;
        *out += 32;
        rounds -= 1;
    }

    // Add the offset back:
    *src += 4;
}

void base64_stream_encode_final(std::size_t carry, std::size_t bytes, char* out,
                                std::size_t& written)
{
    if (bytes == 1)
    {
        *out++ = base64_table_enc_6bit[carry];
        *out++ = '=';
        *out++ = '=';
        written += 3;
        return;
    }
    if (bytes == 2)
    {
        *out++ = base64_table_enc_6bit[carry];
        *out++ = '=';
        written += 2;
        return;
    }
}

std::size_t base64_avx2::encode(const uint8_t* data, std::size_t size,
                                char* out)
{
    std::size_t written = 0;
    std::size_t remaining = size;
    auto bytes = 0;
    auto carry = 0;

    while (true)
    {
        encode_loop_avx2(&data, remaining, &out, written);
        if (remaining-- == 0)
        {
            break;
        }
        *out++ = base64_table_enc_6bit[*data >> 2];
        written += 1;
        carry = (*data++ << 4) & 0x30;
        bytes++;
        if (remaining-- == 0)
        {
            break;
        }
        *out++ = base64_table_enc_6bit[carry | (*data >> 4)];
        written += 1;
        carry = (*data++ << 2) & 0x3C;
        bytes++;
        if (remaining-- == 0)
        {
            break;
        }
        *out++ = base64_table_enc_6bit[carry | (*data >> 6)];
        written += 1;
        *out++ = base64_table_enc_6bit[*data++ & 0x3F];
        written += 1;
        bytes = 0;
    }
    base64_stream_encode_final(carry, bytes, out, written);
    return written;
}
static inline __m256i dec_reshuffle(const __m256i in)
{
    // in, lower lane, bits, upper case are most significant bits, lower
    // case are least significant bits:
    // 00llllll 00kkkkLL 00jjKKKK 00JJJJJJ
    // 00iiiiii 00hhhhII 00ggHHHH 00GGGGGG
    // 00ffffff 00eeeeFF 00ddEEEE 00DDDDDD
    // 00cccccc 00bbbbCC 00aaBBBB 00AAAAAA

    const __m256i merge_ab_and_bc =
        _mm256_maddubs_epi16(in, _mm256_set1_epi32(0x01400140));
    // 0000kkkk LLllllll 0000JJJJ JJjjKKKK
    // 0000hhhh IIiiiiii 0000GGGG GGggHHHH
    // 0000eeee FFffffff 0000DDDD DDddEEEE
    // 0000bbbb CCcccccc 0000AAAA AAaaBBBB

    __m256i out =
        _mm256_madd_epi16(merge_ab_and_bc, _mm256_set1_epi32(0x00011000));
    // 00000000 JJJJJJjj KKKKkkkk LLllllll
    // 00000000 GGGGGGgg HHHHhhhh IIiiiiii
    // 00000000 DDDDDDdd EEEEeeee FFffffff
    // 00000000 AAAAAAaa BBBBbbbb CCcccccc

    // Pack bytes together in each lane:
    out = _mm256_shuffle_epi8(
        out, _mm256_setr_epi8(2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, -1, -1,
                              -1, -1, 2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12,
                              -1, -1, -1, -1));
    // 00000000 00000000 00000000 00000000
    // LLllllll KKKKkkkk JJJJJJjj IIiiiiii
    // HHHHhhhh GGGGGGgg FFffffff EEEEeeee
    // DDDDDDdd CCcccccc BBBBbbbb AAAAAAaa

    // Pack lanes:
    return _mm256_permutevar8x32_epi32(
        out, _mm256_setr_epi32(0, 1, 2, 4, 5, 6, -1, -1));
}
static inline int dec_loop_avx2_inner(const char** s, uint8_t** o,
                                      size_t* rounds)
{
    const __m256i lut_lo = _mm256_setr_epi8(
        0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x13, 0x1A,
        0x1B, 0x1B, 0x1B, 0x1A, 0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x13, 0x1A, 0x1B, 0x1B, 0x1B, 0x1A);

    const __m256i lut_hi = _mm256_setr_epi8(
        0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10);

    const __m256i lut_roll = _mm256_setr_epi8(
        0, 16, 19, 4, -65, -65, -71, -71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 19, 4,
        -65, -65, -71, -71, 0, 0, 0, 0, 0, 0, 0, 0);

    const __m256i mask_2F = _mm256_set1_epi8(0x2F);

    // Load input:
    __m256i str = _mm256_loadu_si256((__m256i*)*s);

    // See the SSSE3 decoder for an explanation of the algorithm.
    const __m256i hi_nibbles =
        _mm256_and_si256(_mm256_srli_epi32(str, 4), mask_2F);
    const __m256i lo_nibbles = _mm256_and_si256(str, mask_2F);
    const __m256i hi = _mm256_shuffle_epi8(lut_hi, hi_nibbles);
    const __m256i lo = _mm256_shuffle_epi8(lut_lo, lo_nibbles);

    if (!_mm256_testz_si256(lo, hi))
    {
        return 0;
    }

    const __m256i eq_2F = _mm256_cmpeq_epi8(str, mask_2F);
    const __m256i roll =
        _mm256_shuffle_epi8(lut_roll, _mm256_add_epi8(eq_2F, hi_nibbles));

    // Now simply add the delta values to the input:
    str = _mm256_add_epi8(str, roll);

    // Reshuffle the input to packed 12-byte output format:
    str = dec_reshuffle(str);

    // Store the output:
    _mm256_storeu_si256((__m256i*)*o, str);

    *s += 32;
    *o += 24;
    *rounds -= 1;

    return 1;
}

static inline void dec_loop_avx2(const char** s, size_t* slen, uint8_t** o,
                                 size_t* olen)
{
    if (*slen < 45)
    {
        return;
    }

    // Process blocks of 32 bytes per round. Because 8 extra zero bytes are
    // written after the output, ensure that there will be at least 13
    // bytes of input data left to cover the gap. (11 data bytes and up to
    // two end-of-string markers.)
    size_t rounds = (*slen - 13) / 32;

    *slen -= rounds * 32; // 32 bytes consumed per round
    *olen += rounds * 24; // 24 bytes produced per round

    do
    {
        if (rounds >= 8)
        {
            if (dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds))
            {
                continue;
            }
            break;
        }
        if (rounds >= 4)
        {
            if (dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds))
            {
                continue;
            }
            break;
        }
        if (rounds >= 2)
        {
            if (dec_loop_avx2_inner(s, o, &rounds) &&
                dec_loop_avx2_inner(s, o, &rounds))
            {
                continue;
            }
            break;
        }
        dec_loop_avx2_inner(s, o, &rounds);
        break;

    } while (rounds > 0);

    // Adjust for any rounds that were skipped:
    *slen += rounds * 32;
    *olen -= rounds * 24;
}

#define BASE64_EOF 1
#define BASE64_AEOF 2

std::size_t base64_avx2::decode(const char* src, std::size_t size, uint8_t* out)
{
    uint8_t q;

    // Use local temporaries to avoid cache thrashing:
    std::size_t written = 0;

    std::size_t carry = 0;

    // Turn four 6-bit numbers into three bytes:
    // out[0] = 11111122
    // out[1] = 22223333
    // out[2] = 33444444

    while (true)
    {
        dec_loop_avx2(&src, &size, &out, &written);
        if (size-- == 0)
        {
            return written;
        }
        if ((q = base64_table_dec_8bit[*src++]) >= 254)
        {
            // Treat character '=' as invalid for byte 0:
            assert(false);
            return 0;
        }
        carry = q << 2;

        if (size-- == 0)
        {
            return written;
        }

        if ((q = base64_table_dec_8bit[*src++]) >= 254)
        {
            // Treat character '=' as invalid for byte 1:
            assert(false);
            return 0;
        }
        *out++ = carry | (q >> 4);
        carry = q << 4;
        written++;

        if (size-- == 0)
        {
            return written;
        }
        if ((q = base64_table_dec_8bit[*src++]) >= 254)
        {
            // When q == 254, the input char is '='.
            if (q == 254)
            {
                // Check if next byte is also '=':
                if (size == 1)
                {
                    size--;
                    q = base64_table_dec_8bit[*src++];
                    assert(q == 254);
                    return written;
                }
            }
            // If we get here, there was an error:
            assert(false);
            return 0;
        }
        *out++ = carry | (q >> 2);
        carry = q << 6;
        written++;

        if (size-- == 0)
        {
            return written;
        }
        q = base64_table_dec_8bit[*src++];
        // When q == 254, the input char is '='.
        if (q == 254)
        {
            return written;
        }
        if (q == 255)
        {
            assert(false);
            return 0;
        }

        *out++ = carry | q;
        carry = 0;
        written++;
    }
}

bool base64_avx2::is_compiled()
{
    return true;
}
#else
std::size_t base64_avx2::encode(const uint8_t*, std::size_t, char*)
{
    assert(0 && "Target platform or compiler does not support this "
                "implementation");
    return 0;
}

std::size_t base64_avx2::decode(const char*, std::size_t, uint8_t*)
{
    assert(0 && "Target platform or compiler does not support this "
                "implementation");
    return 0;
}

bool base64_avx2::is_compiled()
{
    return false;
}
#endif
}
}
