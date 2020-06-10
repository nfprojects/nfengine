/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of memory aligning utilities.
 */

#pragma once

#include "../nfCommon.hpp"
#include "DefaultAllocator.hpp"
#include "../System/Assertion.hpp"

#include <type_traits>
#include <string.h>

namespace NFE {
namespace Common {

/**
 * Collection of various classes for low-level C++ objects memory manipulation.
 */
class MemoryHelpers
{
public:

    /**
     * Move an object from 'source' to 'target' using move constructor & destructor.
     * This is a preferred way if move constructor is available.
     */
    template<typename T>
    NFE_INLINE static
        typename std::enable_if<std::is_move_constructible_v<T>, void>::type
        Move(T* target, T* source)
    {
        new (target) T(std::move(*source));
        source->~T();
    }

    /**
     * Move an object from 'source' to 'target' using move constructor & destructor.
     * This is a preferred way if move constructor is not available.
     */
    template<typename T>
    NFE_INLINE static
        typename std::enable_if<!std::is_move_constructible_v<T>, void>::type
        Move(T* target, T* source)
    {
        new (target) T(*source);
        source->~T();
    }

    /**
     * Move an array of objects (will call move constructor or copy constructor if possible).
     * @note    Source and target memory blocks can overlap.
     */
    template<typename T>
    static void MoveArray(T* target, T* source, size_t numElements)
    {
        if (target == source || numElements == 0)
        {
            // nothing to do
            return;
        }

        if (reinterpret_cast<std::uintptr_t>(target) > reinterpret_cast<std::uintptr_t>(source))
        {
            // move starting from the end in this scenario:
            // source: .....XXXXXXXX.........
            // target: .........XXXXXXXXX....

            for (size_t i = numElements; i-- > 0; )
            {
                Move(target + i, source + i);
            }
        }
        else
        {
            for (size_t i = 0; i < numElements; ++i)
            {
                Move(target + i, source + i);
            }
        }
    }
};


} // namespace Common
} // namespace NFE
