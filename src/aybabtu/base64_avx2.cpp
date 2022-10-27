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

const uint8_t base64_table_enc_6bit[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789"
                                        "+/";

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
    // Translation of the SSSE3 reshuffling algorithm to AVX2. This one
    // works with shifted (4 bytes) input in order to be able to work
    // efficiently in the two 128-bit lanes.

    // Input, bytes MSB to LSB:
    // 0 0 0 0 x w v u t s r q p o n m
    // l k j i h g f e d c b a 0 0 0 0

    const __m256i in =
        _mm256_shuffle_epi8(input, _mm256_set_epi8(10, 11, 9, 10, 7, 8, 6, 7, 4,
                                                   5, 3, 4, 1, 2, 0, 1,

                                                   14, 15, 13, 14, 11, 12, 10,
                                                   11, 8, 9, 7, 8, 5, 6, 4, 5));
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
static inline void enc_loop_avx2_inner_first(const uint8_t** s, uint8_t** o)
{
    // First load is done at s - 0 to not get a segfault:
    __m256i src = _mm256_loadu_si256((__m256i*)*s);

    // Shift by 4 bytes, as required by enc_reshuffle:
    src = _mm256_permutevar8x32_epi32(
        src, _mm256_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6));

    // Reshuffle, translate, store:
    src = enc_reshuffle(src);
    src = enc_translate(src);
    _mm256_storeu_si256((__m256i*)*o, src);

    // Subsequent loads will be done at s - 4, set pointer for next round:
    *s += 20;
    *o += 32;
}

static inline void enc_loop_avx2_inner(const uint8_t** s, uint8_t** o)
{
    // Load input:
    __m256i src = _mm256_loadu_si256((__m256i*)*s);

    // Reshuffle, translate, store:
    src = enc_reshuffle(src);
    src = enc_translate(src);
    _mm256_storeu_si256((__m256i*)*o, src);

    *s += 24;
    *o += 32;
}

static inline void enc_loop_avx2(const uint8_t** s, size_t* slen, uint8_t** o,
                                 size_t* olen)
{
    if (*slen < 32)
    {
        return;
    }

    // Process blocks of 24 bytes at a time. Because blocks are loaded 32
    // bytes at a time an offset of -4, ensure that there will be at least
    // 4 remaining bytes after the last round, so that the final read will
    // not pass beyond the bounds of the input buffer:
    size_t rounds = (*slen - 4) / 24;

    *slen -= rounds * 24; // 24 bytes consumed per round
    *olen += rounds * 32; // 32 bytes produced per round

    // The first loop iteration requires special handling to ensure that
    // the read, which is done at an offset, does not underflow the buffer:
    enc_loop_avx2_inner_first(s, o);
    rounds--;

    while (rounds > 0)
    {
        if (rounds >= 8)
        {
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            rounds -= 8;
            continue;
        }
        if (rounds >= 4)
        {
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            rounds -= 4;
            continue;
        }
        if (rounds >= 2)
        {
            enc_loop_avx2_inner(s, o);
            enc_loop_avx2_inner(s, o);
            rounds -= 2;
            continue;
        }
        enc_loop_avx2_inner(s, o);
        break;
    }

    // Add the offset back:
    *s += 4;
}

void base64_stream_encode_final(size_t carry, size_t bytes, uint8_t* out,
                                size_t* outlen)
{

    char* o = (char*)out;
    if (bytes == 1)
    {
        *o++ = base64_table_enc_6bit[carry];
        *o++ = '=';
        *o++ = '=';
        *outlen += 3;
        return;
    }
    if (bytes == 2)
    {
        *o++ = base64_table_enc_6bit[carry];
        *o++ = '=';
        *outlen += 2;
        return;
    }
}

std::size_t base64_avx2::encode(const uint8_t* data, std::size_t size,
                                char* out)
{
    // Assume that *out is large enough to contain the output.
    // Theoretically it should be 4/3 the length of src.
    uint8_t* o = (uint8_t*)out;

    // Use local temporaries to avoid cache thrashing:
    size_t olen = 0;
    auto bytes = 0;
    auto carry = 0;

    // Turn three bytes into four 6-bit numbers:
    // in[0] = 00111111
    // in[1] = 00112222
    // in[2] = 00222233
    // in[3] = 00333333

    // Duff's device, a for() loop inside a switch() statement. Legal!
    switch (bytes)
    {
        for (;;)
        {
        case 0:

            enc_loop_avx2(&data, &size, &o, &olen);

            if (size-- == 0)
            {
                break;
            }
            *o++ = base64_table_enc_6bit[*data >> 2];
            carry = (*data++ << 4) & 0x30;
            bytes++;
            olen += 1;
        case 1:
            if (size-- == 0)
            {
                break;
            }
            *o++ = base64_table_enc_6bit[carry | (*data >> 4)];
            carry = (*data++ << 2) & 0x3C;
            bytes++;
            olen += 1;
        case 2:
            if (size-- == 0)
            {
                break;
            }
            *o++ = base64_table_enc_6bit[carry | (*data >> 6)];
            *o++ = base64_table_enc_6bit[*data++ & 0x3F];
            bytes = 0;
            olen += 2;
        }
    }
    base64_stream_encode_final(carry, bytes, o, &olen);
    return olen;
}

std::size_t base64_avx2::decode(const char* encoded_string, std::size_t size,
                                uint8_t* data)
{
    return 0;
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
