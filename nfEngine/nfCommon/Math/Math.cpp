/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic math functions definitions.
 */

#include "PCH.hpp"
#include "Math.hpp"


#ifdef NFE_USE_SSE
#pragma message("INFO: Compiling nfCommon with SSE instructions")
#endif // NFE_USE_SSE

#ifdef NFE_USE_SSE4
#pragma message("INFO: Compiling nfCommon with SSE4.1 instructions")
#endif // NFE_USE_SSE4

#ifdef NFE_USE_AVX
#pragma message("INFO: Compiling nfCommon with AVX instructions")
#endif // NFE_USE_AVX

#ifdef NFE_USE_AVX2
#pragma message("INFO: Compiling nfCommon with AVX2 instructions")
#endif // NFE_USE_AVX2

#ifdef NFE_USE_FMA
#pragma message("INFO: Compiling nfCommon with FMA instructions")
#endif // NFE_USE_FMA


namespace NFE {
namespace Math {


static_assert(sizeof(Bits32) == 4, "Invalid size of 'Bits32'");
static_assert(sizeof(Bits64) == 8, "Invalid size of 'Bits64'");


} // namespace Math
} // namespace NFE
