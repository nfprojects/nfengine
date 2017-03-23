/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module for handling MD5 calculations
 */

#pragma once

#include "../nfCommon.hpp"

#include <vector>


// basic MD5 bitwise operations
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

// MD5 shift constants
#define MD5_S11 7
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
#define MD5_S21 5
#define MD5_S22 9
#define MD5_S23 14
#define MD5_S24 20
#define MD5_S31 4
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
#define MD5_S41 6
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21

// MD5 functions
#define MD5_ROT_LEFT(x, c) ( ((x) << (c)) | ((x) >> (32 - (c)) ) )

#define MD5_FF(a, b, c, d, x, s, t) { \
        (a) += (MD5_F((b), (c), (d))) + (x) + (t); \
        (a) = MD5_ROT_LEFT((a), (s)) + (b); }

#define MD5_GG(a, b, c, d, x, s, t) { \
        (a) += (MD5_G((b), (c), (d))) + (x) + (t); \
        (a) = MD5_ROT_LEFT((a), (s)) + (b); }

#define MD5_HH(a, b, c, d, x, s, t) { \
        (a) += (MD5_H((b), (c), (d))) + (x) + (t); \
        (a) = MD5_ROT_LEFT((a), (s)) + (b); }

#define MD5_II(a, b, c, d, x, s, t) { \
        (a) += (MD5_I((b), (c), (d))) + (x) + (t); \
        (a) = MD5_ROT_LEFT((a), (s)) + (b); }

namespace NFE {
namespace Common {

struct NFCOMMON_API MD5DataSet
{
    MD5DataSet(const String& input);
    ~MD5DataSet();

    std::vector<uint32> mChunks;
};

class NFCOMMON_API MD5Hash
{
public:
    MD5Hash();

    void Calculate(const String& data);
    void Set(uint32 h0, uint32 h1, uint32 h2, uint32 h3);

    // operators
    bool operator==(const MD5Hash& hash) const;
    uint32 operator[] (int i) const;

    // friend operators
    friend NFCOMMON_API std::ostream& operator<<(std::ostream& os, const MD5Hash& hash);
    friend NFCOMMON_API std::istream& operator>>(std::istream& is, MD5Hash& hash);

    // for debugging purposes
    void Print() const;

private:
    // TODO Investigate if speeding up MD5 calculations with SSE is possible
    uint32 h[4];
};

// friend operators
NFCOMMON_API std::ostream& operator<<(std::ostream& os, const MD5Hash& hash);
NFCOMMON_API std::istream& operator>>(std::istream& is, MD5Hash& hash);

} // namespace Common
} // namespace NFE
