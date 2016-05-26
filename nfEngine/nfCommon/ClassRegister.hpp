/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Generic memory allocator declaration.
 */

#pragma once
#include "nfCommon.hpp"
#include "Memory/GenericAllocator.hpp"

namespace NFE {
namespace Common {

enum class ClassAllocatorType
{
    Generic,
    Pool
};

struct ClassInfo
{
    const char* name;
    size_t size;
    size_t alignment;
};

class NFCOMMON_API ClassRegister
{
    static std::vector<ClassInfo> classes;

public:
    static size_t Register(const char* name, size_t size, size_t alignment);
};


} // namespace Common
} // namespace NFE


#define NFE_DECLARE_CLASS                       \
void* operator new(size_t size);                \
void operator delete(void* ptr);                \
void* operator new[](size_t size);              \
void operator delete[](void* ptr);              \
static size_t mClassId;


#define NFE_DEFINE_CLASS(name, alignment, allocatorType)                                        \
size_t name::mClassId = NFE::Common::ClassRegister::Register(#name, sizeof(name), alignment);   \
void* name::operator new(size_t size)                                                           \
{                                                                                               \
    return NFE_GENERIC_MALLOC(size, alignment);                                                 \
}                                                                                               \
void name::operator delete(void* ptr)                                                           \
{                                                                                               \
    NFE_GENERIC_FREE(ptr);                                                                      \
}                                                                                               \
void* name::operator new[](size_t size)                                                         \
{                                                                                               \
    return NFE_GENERIC_MALLOC(size, alignment);                                                 \
}                                                                                               \
void name::operator delete[](void* ptr)                                                         \
{                                                                                               \
    NFE_GENERIC_FREE(ptr);                                                                      \
}
