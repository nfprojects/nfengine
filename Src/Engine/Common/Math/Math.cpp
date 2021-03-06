#include "PCH.hpp"
#include "Math.hpp"

#ifdef NFE_USE_SSE
#pragma message("[NFE::Math] Compiling with SSE support")
#endif

#ifdef NFE_USE_AVX
#pragma message("[NFE::Math] Compiling with AVX support")
#endif

#ifdef NFE_USE_AVX2
#pragma message("[NFE::Math] Compiling with AVX2 support")
#endif

#ifdef NFE_USE_AVX512
#pragma message("[NFE::Math] Compiling with AVX-512 support")
#endif

#ifdef NFE_USE_FP16C
#pragma message("[NFE::Math] Compiling with FP16C support")
#endif

#ifdef NFE_USE_FMA
#pragma message("[NFE::Math] Compiling with FMA support")
#endif

namespace NFE {
namespace Math {

void SetFlushDenormalsToZero(bool enable)
{
    NFE_UNUSED(enable);
#ifdef NFE_USE_SSE
    _MM_SET_DENORMALS_ZERO_MODE(enable ? _MM_DENORMALS_ZERO_ON : _MM_DENORMALS_ZERO_OFF);
    _MM_SET_FLUSH_ZERO_MODE(enable ? _MM_FLUSH_ZERO_ON : _MM_FLUSH_ZERO_OFF);
#endif // NFE_USE_SSE
}

bool GetFlushDenormalsToZero()
{
#ifdef NFE_USE_SSE
    return _MM_GET_DENORMALS_ZERO_MODE() && _MM_GET_FLUSH_ZERO_MODE();
#else
    return true;
#endif // NFE_USE_SSE
}

} // namespace Math
} // namespace NFE
