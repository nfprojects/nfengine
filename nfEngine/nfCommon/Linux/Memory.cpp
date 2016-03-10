/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Memory utilities definitions
 */

#include "../PCH.hpp"
#include "../Memory.hpp"

#include <sys/file.h>

namespace NFE {
namespace Common {

bool MemoryCheck(const void* ptr, size_t size)
{
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    int fd = ::open("/proc/self/mem", O_RDONLY);

    if (fd < 0)
        return false;

    size_t offset = reinterpret_cast<size_t>(ptr);
    ::lseek(fd, offset, SEEK_SET);

    while (size > BUFFER_SIZE)
    {
        if (::read(fd, buffer, BUFFER_SIZE) != BUFFER_SIZE)
            goto fail;
        size -= BUFFER_SIZE;
    }

    if (::read(fd, buffer, size) != size)
        goto fail;

    ::close(fd);
    return true;

fail:
    ::close(fd);
    return false;
}


} // namespace Common
} // namespace NFE
