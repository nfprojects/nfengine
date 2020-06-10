#include "PCH.hpp"
#include "HilbertCurve.hpp"

namespace NFE {
namespace Math {

void HilbertIndexToCoords(uint32 size, uint32 index, uint32& x, uint32& y)
{
#ifdef NFE_USE_AVX
    NFE_UNUSED(size);

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
    uint32 rx, ry, t = index;
    for (uint32 s = 1; s < size; s *= 2u)
    {
        rx = 1 & (t / 2);
        ry = 1 & (t ^ rx);

        if (ry == 0)
        {
            if (rx == 1)
            {
                x = s - 1 - x;
                y = s - 1 - y;
            }
            std::swap(x, y);
        }

        x += s * rx;
        y += s * ry;
        t /= 4u;
    }

#endif
}

} // namespace Math
} // namespace NFE
