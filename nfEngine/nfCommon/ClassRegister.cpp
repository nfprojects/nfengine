/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Generic memory allocator definition.
 */

#include "PCH.hpp"
#include "ClassRegister.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

std::vector<ClassInfo> ClassRegister::classes;

size_t ClassRegister::Register(const char* name, size_t size, size_t alignment)
{
    size_t id = classes.size();

    LOG_DEBUG("Registering class '%s': ID=%zu, size=%zu, alignment=%zu", name, id, size, alignment);

    ClassInfo info;
    info.name = name;
    info.size = size;
    info.alignment = alignment;
    classes.push_back(info);

    return id;
}

} // namespace Common
} // namespace NFE
