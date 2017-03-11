/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Memory utilities definitions
 */

#include "PCH.hpp"
#include "../Memory.hpp"

#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>


namespace NFE {
namespace Common {

bool MemoryCheck(const void* ptr, size_t size)
{
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    bool result = true;
    int fd = ::open("/proc/self/mem", O_RDONLY);

    if (fd < 0)
        return false;

    size_t offset = reinterpret_cast<size_t>(ptr);
    ::lseek(fd, offset, SEEK_SET);

    while (size > BUFFER_SIZE)
    {
        if (::read(fd, buffer, BUFFER_SIZE) != BUFFER_SIZE)
        {
            result = false;
            goto ret;
        }
        size -= BUFFER_SIZE;
    }

    if (::read(fd, buffer, size) != size)
    {
        result = false;
        goto ret;
    }

ret:
    ::close(fd);
    return result;
}


} // namespace Common
} // namespace NFE
