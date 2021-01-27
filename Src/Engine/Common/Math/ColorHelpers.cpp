/**
 * @file
 * @author Witek902
 */

#include "PCH.hpp"
#include "ColorHelpers.hpp"

namespace NFE {
namespace Math {

static const float* Init_sRGB_To_Linear_LUT()
{
	float* lut = (float*)NFE_MALLOC(256 * sizeof(float), NFE_CACHE_LINE_SIZE);

	NFE_ASSERT(lut, "Failed to allocate LUT");

	for (uint32 i = 0; i < 256u; ++i)
	{
		const float x = static_cast<float>(i) / 255.0f;
		lut[i] = Convert_sRGB_To_Linear_Exact(x);
	}

	return lut;
}

NFCOMMON_API const float* sRGB_To_Linear_LUT = Init_sRGB_To_Linear_LUT();

} // namespace Math
} // namespace NFE
