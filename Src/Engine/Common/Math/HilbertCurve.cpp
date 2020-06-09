#include "PCH.hpp"
#include "HilbertCurve.hpp"

namespace NFE {
namespace Math {

void HilbertIndexToCoords(uint32 index, uint32& x, uint32& y)
{
#ifdef NFE_USE_AVX
    // source: http://threadlocalmutex.com/?p=188

    uint32 i0 = _pext_u32(index, 0x55555555);
    uint32 i1 = _pext_u32(index, 0xAAAAAAAA);

    uint32 A = i0 & i1;
    uint32 B = i0 ^ i1 ^ 0xFFFF;

    uint32 C = uint32(_mm_extract_epi16(_mm_clmulepi64_si128(_mm_set_epi64x(0, 0xFFFF), _mm_cvtsi32_si128(A), 0b00), 1));
    uint32 D = uint32(_mm_extract_epi16(_mm_clmulepi64_si128(_mm_set_epi64x(0, 0xFFFF), _mm_cvtsi32_si128(B), 0b00), 1));

    uint32 a = C ^ (i0 & D);

    x = (a ^ i1);
    y = (a ^ i0 ^ i1);
#else
    x = y = 0;
    uint32 k = 0;
    do
    {
        uint32 rx = (index >> 1) & 1u;
        uint32 ry = (index ^ (index >> 1)) & 1u;
        x += rx << k;
        y += ry << k;
        index >>= 2;
    }
    while (index > 0u);
#endif
}

} // namespace Math
} // namespace NFE
