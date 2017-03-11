/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Module for handling MD5 calculations
 */

#include "PCH.hpp"
#include "MD5.hpp"
#include "Bit.hpp"

#include <iomanip>


namespace NFE {
namespace Common {

MD5DataSet::MD5DataSet(const std::string& input)
{
    if (input.size() == 0)
    {
        // no data provided - initialize set in faster, simplier way
        mChunks.resize(16);

        mChunks[0] = 0x00000080;

        for (int i = 1; i < 16; i++)
            mChunks[i] = 0;

        return;
    }

    size_t mChunkSize;
    if (input.size() % 64 >= 56)
    {
        mChunkSize = ((input.size() / 64) + 2) * 16;
    }
    else
    {
        mChunkSize = ((input.size() / 64) + 1) * 16;
    }

    // initialization - allocate memory and zero it
    mChunks.resize(mChunkSize);
    memset(mChunks.data(), 0, mChunkSize * sizeof(uint32));

    // copy data
    memcpy(mChunks.data(), input.c_str(), input.size());

    // append '1' bit to the end of data
    size_t d = input.size() / 4;
    uint32 mod = input.size() % 4;
    mChunks[d] += 0x80 << mod * 8;

    // append size of message in bits to end of data
    uint64 bitDataLength = input.size() * 8;

    mChunks[mChunkSize - 2] = (uint32)(bitDataLength);
    mChunks[mChunkSize - 1] = (uint32)(bitDataLength >> 32);
}

MD5DataSet::~MD5DataSet() {}



MD5Hash::MD5Hash()
{
    h[0] = 0x67452301;
    h[1] = 0xefcdab89;
    h[2] = 0x98badcfe;
    h[3] = 0x10325476;
}

void MD5Hash::Calculate(const std::string& input)
{
    MD5DataSet data(input);

    // do complete round of MD5 calculation for each set of 16 chunks
    for (unsigned int i = 0; i < data.mChunks.size(); i += 16)
    {
        uint32 a = h[0];
        uint32 b = h[1];
        uint32 c = h[2];
        uint32 d = h[3];



        MD5_FF(a, b, c, d, data.mChunks[   i], MD5_S11, 0xd76aa478);
        MD5_FF(d, a, b, c, data.mChunks[ 1 + i], MD5_S12, 0xe8c7b756);
        MD5_FF(c, d, a, b, data.mChunks[ 2 + i], MD5_S13, 0x242070db);
        MD5_FF(b, c, d, a, data.mChunks[ 3 + i], MD5_S14, 0xc1bdceee);

        MD5_FF(a, b, c, d, data.mChunks[ 4 + i], MD5_S11, 0xf57c0faf);
        MD5_FF(d, a, b, c, data.mChunks[ 5 + i], MD5_S12, 0x4787c62a);
        MD5_FF(c, d, a, b, data.mChunks[ 6 + i], MD5_S13, 0xa8304613);
        MD5_FF(b, c, d, a, data.mChunks[ 7 + i], MD5_S14, 0xfd469501);

        MD5_FF(a, b, c, d, data.mChunks[ 8 + i], MD5_S11, 0x698098d8);
        MD5_FF(d, a, b, c, data.mChunks[ 9 + i], MD5_S12, 0x8b44f7af);
        MD5_FF(c, d, a, b, data.mChunks[10 + i], MD5_S13, 0xffff5bb1);
        MD5_FF(b, c, d, a, data.mChunks[11 + i], MD5_S14, 0x895cd7be);

        MD5_FF(a, b, c, d, data.mChunks[12 + i], MD5_S11, 0x6b901122);
        MD5_FF(d, a, b, c, data.mChunks[13 + i], MD5_S12, 0xfd987193);
        MD5_FF(c, d, a, b, data.mChunks[14 + i], MD5_S13, 0xa679438e);
        MD5_FF(b, c, d, a, data.mChunks[15 + i], MD5_S14, 0x49b40821);



        MD5_GG(a, b, c, d, data.mChunks[ 1 + i], MD5_S21, 0xf61e2562);
        MD5_GG(d, a, b, c, data.mChunks[ 6 + i], MD5_S22, 0xc040b340);
        MD5_GG(c, d, a, b, data.mChunks[11 + i], MD5_S23, 0x265e5a51);
        MD5_GG(b, c, d, a, data.mChunks[   i], MD5_S24, 0xe9b6c7aa);

        MD5_GG(a, b, c, d, data.mChunks[ 5 + i], MD5_S21, 0xd62f105d);
        MD5_GG(d, a, b, c, data.mChunks[10 + i], MD5_S22, 0x02441453);
        MD5_GG(c, d, a, b, data.mChunks[15 + i], MD5_S23, 0xd8a1e681);
        MD5_GG(b, c, d, a, data.mChunks[ 4 + i], MD5_S24, 0xe7d3fbc8);

        MD5_GG(a, b, c, d, data.mChunks[ 9 + i], MD5_S21, 0x21e1cde6);
        MD5_GG(d, a, b, c, data.mChunks[14 + i], MD5_S22, 0xc33707d6);
        MD5_GG(c, d, a, b, data.mChunks[ 3 + i], MD5_S23, 0xf4d50d87);
        MD5_GG(b, c, d, a, data.mChunks[ 8 + i], MD5_S24, 0x455a14ed);

        MD5_GG(a, b, c, d, data.mChunks[13 + i], MD5_S21, 0xa9e3e905);
        MD5_GG(d, a, b, c, data.mChunks[ 2 + i], MD5_S22, 0xfcefa3f8);
        MD5_GG(c, d, a, b, data.mChunks[ 7 + i], MD5_S23, 0x676f02d9);
        MD5_GG(b, c, d, a, data.mChunks[12 + i], MD5_S24, 0x8d2a4c8a);



        MD5_HH(a, b, c, d, data.mChunks[ 5 + i], MD5_S31, 0xfffa3942);
        MD5_HH(d, a, b, c, data.mChunks[ 8 + i], MD5_S32, 0x8771f681);
        MD5_HH(c, d, a, b, data.mChunks[11 + i], MD5_S33, 0x6d9d6122);
        MD5_HH(b, c, d, a, data.mChunks[14 + i], MD5_S34, 0xfde5380c);

        MD5_HH(a, b, c, d, data.mChunks[ 1 + i], MD5_S31, 0xa4beea44);
        MD5_HH(d, a, b, c, data.mChunks[ 4 + i], MD5_S32, 0x4bdecfa9);
        MD5_HH(c, d, a, b, data.mChunks[ 7 + i], MD5_S33, 0xf6bb4b60);
        MD5_HH(b, c, d, a, data.mChunks[10 + i], MD5_S34, 0xbebfbc70);

        MD5_HH(a, b, c, d, data.mChunks[13 + i], MD5_S31, 0x289b7ec6);
        MD5_HH(d, a, b, c, data.mChunks[   i], MD5_S32, 0xeaa127fa);
        MD5_HH(c, d, a, b, data.mChunks[ 3 + i], MD5_S33, 0xd4ef3085);
        MD5_HH(b, c, d, a, data.mChunks[ 6 + i], MD5_S34, 0x04881d05);

        MD5_HH(a, b, c, d, data.mChunks[ 9 + i], MD5_S31, 0xd9d4d039);
        MD5_HH(d, a, b, c, data.mChunks[12 + i], MD5_S32, 0xe6db99e5);
        MD5_HH(c, d, a, b, data.mChunks[15 + i], MD5_S33, 0x1fa27cf8);
        MD5_HH(b, c, d, a, data.mChunks[ 2 + i], MD5_S34, 0xc4ac5665);



        MD5_II(a, b, c, d, data.mChunks[   i], MD5_S41, 0xf4292244);
        MD5_II(d, a, b, c, data.mChunks[ 7 + i], MD5_S42, 0x432aff97);
        MD5_II(c, d, a, b, data.mChunks[14 + i], MD5_S43, 0xab9423a7);
        MD5_II(b, c, d, a, data.mChunks[ 5 + i], MD5_S44, 0xfc93a039);

        MD5_II(a, b, c, d, data.mChunks[12 + i], MD5_S41, 0x655b59c3);
        MD5_II(d, a, b, c, data.mChunks[ 3 + i], MD5_S42, 0x8f0ccc92);
        MD5_II(c, d, a, b, data.mChunks[10 + i], MD5_S43, 0xffeff47d);
        MD5_II(b, c, d, a, data.mChunks[ 1 + i], MD5_S44, 0x85845dd1);

        MD5_II(a, b, c, d, data.mChunks[ 8 + i], MD5_S41, 0x6fa87e4f);
        MD5_II(d, a, b, c, data.mChunks[15 + i], MD5_S42, 0xfe2ce6e0);
        MD5_II(c, d, a, b, data.mChunks[ 6 + i], MD5_S43, 0xa3014314);
        MD5_II(b, c, d, a, data.mChunks[13 + i], MD5_S44, 0x4e0811a1);

        MD5_II(a, b, c, d, data.mChunks[ 4 + i], MD5_S41, 0xf7537e82);
        MD5_II(d, a, b, c, data.mChunks[11 + i], MD5_S42, 0xbd3af235);
        MD5_II(c, d, a, b, data.mChunks[ 2 + i], MD5_S43, 0x2ad7d2bb);
        MD5_II(b, c, d, a, data.mChunks[ 9 + i], MD5_S44, 0xeb86d391);



        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
    }
}

void MD5Hash::Set(uint32 h0, uint32 h1, uint32 h2, uint32 h3)
{
    h[0] = h0;
    h[1] = h1;
    h[2] = h2;
    h[3] = h3;
}

bool MD5Hash::operator==(const MD5Hash& hash) const
{
    return ((this->h[0] == hash.h[0]) && (this->h[1] == hash.h[1]) && (this->h[2] == hash.h[2]) &&
            (this->h[3] == hash.h[3]));
}

uint32 MD5Hash::operator[] (int i) const
{
    return h[i];
}


// friend operators
std::ostream& operator<<(std::ostream& os, const MD5Hash& hash)
{
    // modify endianness for display purposes
    MD5Hash dispHash = hash;
    SWAP_ENDIANNESS(dispHash.h[0]);
    SWAP_ENDIANNESS(dispHash.h[1]);
    SWAP_ENDIANNESS(dispHash.h[2]);
    SWAP_ENDIANNESS(dispHash.h[3]);

    // do the display operation
    os.setf(std::ios_base::hex, std::ios_base::basefield);
    os << std::setfill('0');
    os << std::setw(8) << dispHash.h[0] << std::setw(8) << dispHash.h[1] << std::setw(8)
       << dispHash.h[2] << std::setw(8) << dispHash.h[3];
    os.unsetf(std::ios_base::hex);
    return os;
}

std::istream& operator>>(std::istream& is, MD5Hash& hash)
{
    is >> hash.h[0] >> hash.h[1] >> hash.h[2] >> hash.h[3];
    return is;
}

} // namespace Common
} // namespace NFE
