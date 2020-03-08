#pragma once

#include "../nfCommon.hpp"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Wincrypt.h>
#endif // WIN32

namespace NFE {
namespace Common {

// True random number generator (slow)
class NFCOMMON_API Entropy
{
    NFE_MAKE_NONCOPYABLE(Entropy);
    NFE_MAKE_NONMOVEABLE(Entropy);

public:
    Entropy();
    ~Entropy();

    uint32 GetInt();

private:
#if defined(WIN32)
    HCRYPTPROV mCryptProv;
#elif defined(__LINUX__) | defined(__linux__)
    int mRandomSourceFD;
#endif // defined(WIN32)
};

} // namespace Common
} // namespace NFE
