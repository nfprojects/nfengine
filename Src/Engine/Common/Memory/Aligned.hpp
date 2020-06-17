/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of memory aligning utilities.
 */

#pragma once

#include "DefaultAllocator.hpp"


#define NFE_ALIGNED_CLASS(Alignment)                                                                                \
    NFE_FORCE_INLINE void* operator new(size_t size)                                                                \
    {                                                                                                               \
        return NFE_MALLOC(size, Alignment);                                                                         \
    }                                                                                                               \
    NFE_FORCE_INLINE void operator delete(void* ptr)                                                                \
    {                                                                                                               \
        NFE_FREE(ptr);                                                                                              \
    }                                                                                                               \
    NFE_FORCE_INLINE void* operator new[](size_t size)                                                              \
    {                                                                                                               \
        return NFE_MALLOC(size, Alignment);                                                                         \
    }                                                                                                               \
    NFE_FORCE_INLINE void operator delete[](void* ptr)                                                              \
    {                                                                                                               \
        NFE_FREE(ptr);                                                                                              \
    }                                                                                                               \
    NFE_FORCE_INLINE void* operator new(size_t size, void* ptr)                                                     \
    {                                                                                                               \
        NFE_UNUSED(size);                                                                                           \
        return ptr;                                                                                                 \
    }                                                                                                               \
    NFE_FORCE_INLINE void* operator new[](size_t size, void* ptr)                                                   \
    {                                                                                                               \
        NFE_UNUSED(size);                                                                                           \
        return ptr;                                                                                                 \
    }                                                                                                               \
    NFE_FORCE_INLINE void operator delete(void* ptr, void* place)                                                   \
    {                                                                                                               \
        NFE_UNUSED(ptr);                                                                                            \
        NFE_UNUSED(place);                                                                                          \
    }                                                                                                               \
    NFE_FORCE_INLINE void operator delete[](void* ptr, void* place)                                                 \
    {                                                                                                               \
        NFE_UNUSED(ptr);                                                                                            \
        NFE_UNUSED(place);                                                                                          \
    }
