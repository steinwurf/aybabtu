// Copyright (c) Steinwurf ApS 2016.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include "base64_neon.hpp"
#include "base64_decode.hpp"
#include "base64_encode.hpp"

#include "../version.hpp"

#include <platform/config.hpp>

#include <cassert>
#include <cstdint>

// Include ARM NEON intrinsics
#if defined(PLATFORM_NEON)
#include <arm_neon.h>
#endif

namespace aybabtu
{
inline namespace STEINWURF_AYBABTU_VERSION
{
namespace detail
{
#ifdef PLATFORM_NEON

// This code borrows from code from Alfred Klomp's library
// https://github.com/aklomp/base64 (published under BSD)
// The code has been modified to fit the aybabtu library.

static inline uint8x16x4_t enc_reshuffle(uint8x16x3_t in)
{
    uint8x16x4_t out;

    // Input:
    // in[0]  = a7 a6 a5 a4 a3 a2 a1 a0
    // in[1]  = b7 b6 b5 b4 b3 b2 b1 b0
    // in[2]  = c7 c6 c5 c4 c3 c2 c1 c0

    // Output:
    // out[0] = 00 00 a7 a6 a5 a4 a3 a2
    // out[1] = 00 00 a1 a0 b7 b6 b5 b4
    // out[2] = 00 00 b3 b2 b1 b0 c7 c6
    // out[3] = 00 00 c5 c4 c3 c2 c1 c0

    // Move the input bits to where they need to be in the outputs. Except
    // for the first output, the high two bits are not cleared.
    out.val[0] = vshrq_n_u8(in.val[0], 2);
    out.val[1] = vshrq_n_u8(in.val[1], 4);
    out.val[2] = vshrq_n_u8(in.val[2], 6);
    out.val[1] = vsliq_n_u8(out.val[1], in.val[0], 4);
    out.val[2] = vsliq_n_u8(out.val[2], in.val[1], 2);

    // Clear the high two bits in the second, third and fourth output.
    out.val[1] = vandq_u8(out.val[1], vdupq_n_u8(0x3F));
    out.val[2] = vandq_u8(out.val[2], vdupq_n_u8(0x3F));
    out.val[3] = vandq_u8(in.val[2], vdupq_n_u8(0x3F));

    return out;
}

// A lookup table containing the absolute offsets for all ranges:
const uint8x16_t enc_translate_lut = {65U,  71U,  252U, 252U, 252U, 252U,
                                      252U, 252U, 252U, 252U, 252U, 252U,
                                      237U, 240U, 0U,   0U};

static inline uint8x16x4_t enc_translate(const uint8x16x4_t in)
{

    const uint8x16_t offset = vdupq_n_u8(51);

    uint8x16x4_t indices, mask, delta, out;

    // Translate values 0..63 to the Base64 alphabet. There are five sets:
    // #  From      To         Abs    Index  Characters
    // 0  [0..25]   [65..90]   +65        0  ABCDEFGHIJKLMNOPQRSTUVWXYZ
    // 1  [26..51]  [97..122]  +71        1  abcdefghijklmnopqrstuvwxyz
    // 2  [52..61]  [48..57]    -4  [2..11]  0123456789
    // 3  [62]      [43]       -19       12  +
    // 4  [63]      [47]       -16       13  /

    // Create LUT indices from input:
    // the index for range #0 is right, others are 1 less than expected:
    indices.val[0] = vqsubq_u8(in.val[0], offset);
    indices.val[1] = vqsubq_u8(in.val[1], offset);
    indices.val[2] = vqsubq_u8(in.val[2], offset);
    indices.val[3] = vqsubq_u8(in.val[3], offset);

    // mask is 0xFF (-1) for range #[1..4] and 0x00 for range #0:
    mask.val[0] = vcgtq_u8(in.val[0], vdupq_n_u8(25));
    mask.val[1] = vcgtq_u8(in.val[1], vdupq_n_u8(25));
    mask.val[2] = vcgtq_u8(in.val[2], vdupq_n_u8(25));
    mask.val[3] = vcgtq_u8(in.val[3], vdupq_n_u8(25));

    // Subtract -1, so add 1 to indices for range #[1..4], All indices are
    // now correct:
    indices.val[0] = vsubq_u8(indices.val[0], mask.val[0]);
    indices.val[1] = vsubq_u8(indices.val[1], mask.val[1]);
    indices.val[2] = vsubq_u8(indices.val[2], mask.val[2]);
    indices.val[3] = vsubq_u8(indices.val[3], mask.val[3]);

    // Lookup delta values:
    delta.val[0] = vqtbl1q_u8(enc_translate_lut, indices.val[0]);
    delta.val[1] = vqtbl1q_u8(enc_translate_lut, indices.val[1]);
    delta.val[2] = vqtbl1q_u8(enc_translate_lut, indices.val[2]);
    delta.val[3] = vqtbl1q_u8(enc_translate_lut, indices.val[3]);

    // Add delta values:
    out.val[0] = vaddq_u8(in.val[0], delta.val[0]);
    out.val[1] = vaddq_u8(in.val[1], delta.val[1]);
    out.val[2] = vaddq_u8(in.val[2], delta.val[2]);
    out.val[3] = vaddq_u8(in.val[3], delta.val[3]);

    return out;
}

static inline void encode_loop_neon(const uint8_t** src, std::size_t& remaining,
                                    uint8_t** out, std::size_t& written)
{
    std::size_t rounds = remaining / 48;

    remaining -= rounds * 48; // 48 bytes consumed per round
    written += rounds * 64;   // 64 bytes produced per round

    while (rounds > 0)
    {
        // Load 48 bytes and deinterleave:
        uint8x16x3_t neon_src = vld3q_u8(*src);

        // Reshuffle:
        uint8x16x4_t neon_out = enc_reshuffle(neon_src);

        // Translate reshuffled bytes to the Base64 alphabet:
        neon_out = enc_translate(neon_out);

        // Interleave and store output:
        vst4q_u8(*out, neon_out);

        *src += 48;
        *out += 64;
        rounds--;
    }
}

static inline int is_nonzero(const uint8x16_t v)
{
    uint64_t u64;
    const uint64x2_t v64 = vreinterpretq_u64_u8(v);
    const uint32x2_t v32 = vqmovn_u64(v64);

    vst1_u64(&u64, vreinterpret_u64_u32(v32));
    return u64 != 0;
}

const uint8x8_t delta_lookup_lut = {
    0, 16, 19, 4, (uint8_t)-65, (uint8_t)-65, (uint8_t)-71, (uint8_t)-71,
};
static inline uint8x16_t delta_lookup(const uint8x16_t v)
{

    return vcombine_u8(vtbl1_u8(delta_lookup_lut, vget_low_u8(v)),
                       vtbl1_u8(delta_lookup_lut, vget_high_u8(v)));
}

const uint8x16_t lane_lut_lo = {0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                                0x11, 0x11, 0x13, 0x1A, 0x1B, 0x1B, 0x1B, 0x1A};

const uint8x16_t lane_lut_hi = {0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08,
                                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
static inline uint8x16_t dec_loop_neon32_lane(uint8x16_t* lane)
{
    // See the SSSE3 decoder for an explanation of the algorithm.
    const uint8x16_t mask_0F = vdupq_n_u8(0x0F);
    const uint8x16_t mask_2F = vdupq_n_u8(0x2F);

    const uint8x16_t hi_nibbles = vshrq_n_u8(*lane, 4);
    const uint8x16_t lo_nibbles = vandq_u8(*lane, mask_0F);
    const uint8x16_t eq_2F = vceqq_u8(*lane, mask_2F);

    const uint8x16_t hi = vqtbl1q_u8(lane_lut_hi, hi_nibbles);
    const uint8x16_t lo = vqtbl1q_u8(lane_lut_lo, lo_nibbles);

    // Now simply add the delta values to the input:
    *lane = vaddq_u8(*lane, delta_lookup(vaddq_u8(eq_2F, hi_nibbles)));

    // Return the validity mask:
    return vandq_u8(lo, hi);
}
static inline void decode_loop_neon(const uint8_t** src, std::size_t& remaining,
                                    uint8_t** out, std::size_t& written)
{
    if (remaining < 64)
    {
        return;
    }

    // Process blocks of 64 bytes per round. Unlike the SSE codecs, no
    // extra trailing zero bytes are written, so it is not necessary to
    // reserve extra input bytes:
    std::size_t rounds = remaining / 64;

    while (rounds > 0)
    {

        // Load 64 bytes and deinterleave:
        uint8x16x4_t str = vld4q_u8(*src);

        // Decode each lane, collect a mask of invalid inputs:
        const uint8x16_t classified = dec_loop_neon32_lane(&str.val[0]) |
                                      dec_loop_neon32_lane(&str.val[1]) |
                                      dec_loop_neon32_lane(&str.val[2]) |
                                      dec_loop_neon32_lane(&str.val[3]);

        // Check for invalid input: if any of the delta values are
        // zero, fall back on bytewise code to do error checking and
        // reporting:
        if (is_nonzero(classified))
        {
            break;
        }

        remaining -= 64; // 64 bytes consumed per round
        written += 48;   // 48 bytes produced per round

        uint8x16x3_t dec;
        // Compress four bytes into three:
        dec.val[0] =
            vorrq_u8(vshlq_n_u8(str.val[0], 2), vshrq_n_u8(str.val[1], 4));
        dec.val[1] =
            vorrq_u8(vshlq_n_u8(str.val[1], 4), vshrq_n_u8(str.val[2], 2));
        dec.val[2] = vorrq_u8(vshlq_n_u8(str.val[2], 6), str.val[3]);

        // Interleave and store decoded result:
        vst3q_u8(*out, dec);

        *src += 64;
        *out += 48;
        --rounds;
    }
}

std::size_t base64_neon::encode(const uint8_t* src, std::size_t size,
                                uint8_t* out)
{
    return base64_encode(&encode_loop_neon, src, size, out);
}

std::size_t base64_neon::decode(const uint8_t* src, std::size_t size,
                                uint8_t* out)
{
    return base64_decode(&decode_loop_neon, src, size, out);
}

bool base64_neon::is_compiled()
{
    return true;
}
#else
std::size_t base64_neon::encode(const uint8_t*, std::size_t, uint8_t*)
{
    assert(0 && "Target platform or compiler does not support this "
                "implementation");
    return 0;
}

std::size_t base64_neon::decode(const uint8_t*, std::size_t, uint8_t*)
{
    assert(0 && "Target platform or compiler does not support this "
                "implementation");
    return 0;
}

bool base64_neon::is_compiled()
{
    return false;
}
#endif
}
}
}
