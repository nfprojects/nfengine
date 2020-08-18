#include "PCH.hpp"
#include "Entropy.hpp"
#include "../Logger/Logger.hpp"

#if defined(NFE_PLATFORM_LINUX)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif // NFE_PLATFORM_LINUX

namespace NFE {
namespace Common {

Entropy::Entropy()
{
#if defined(NFE_PLATFORM_WINDOWS)

    mCryptProv = NULL;
    const LPCSTR userName = "MyKeyContainer";
    if (!::CryptAcquireContextA(&mCryptProv, userName, NULL, PROV_RSA_FULL, 0))
    {
        uint32 errorCode = GetLastError();
        if (::GetLastError() == NTE_BAD_KEYSET)
        {
            if (!::CryptAcquireContextA(&mCryptProv, userName, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
            {
                errorCode = ::GetLastError();
                NFE_LOG_ERROR("Failed to create key container. Error code: %u", errorCode);
            }
        }
        else
        {
            NFE_LOG_ERROR("A cryptographic service handle could not be acquired. Error code: %u", errorCode);
        }

    }

#elif defined(NFE_PLATFORM_LINUX)

    mRandomSourceFD = ::open("/dev/urandom", O_RDONLY);
    if (mRandomSourceFD == -1)
    {
        NFE_LOG_ERROR("Failed to open /dev/urandom, error code: %u", errno);
    }

#else
#error Invalid platform
#endif
}

Entropy::~Entropy()
{
#if defined(NFE_PLATFORM_WINDOWS)

    ::CryptReleaseContext(mCryptProv, 0);

#elif defined(NFE_PLATFORM_LINUX)

    if (mRandomSourceFD != -1)
    {
        ::close(mRandomSourceFD);
    }

#else
#error Invalid platform
#endif
}

uint32 Entropy::GetInt()
{
    uint32 result = 0;

#if defined(NFE_PLATFORM_WINDOWS)

    ::CryptGenRandom(mCryptProv, sizeof(result), (BYTE*)&result);

#elif defined(NFE_PLATFORM_LINUX)

    if (mRandomSourceFD != -1)
    {
        ::read(mRandomSourceFD, &result, sizeof(result));
    }

#else
#error Invalid platform
#endif

    // TODO combine with RTRND?

    return result;
}

} // namespace Common
} // namespace NFE
