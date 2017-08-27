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
 * Collection of various classes for C++ objects memory manipulation.
 */
class MemoryHelpers
{
public:
    template<typename T>
    static constexpr bool isMoveConstructible = std::is_move_constructible<T>::value;

    template<typename T>
    static constexpr bool isCopyConstructible = std::is_copy_constructible<T>::value;

    /**
    * Move an object from 'source' to 'target' using move constructor & destructor.
    * This is a preferred way if move constructor is available.
    */
    template<typename T>
    NFE_INLINE static
        typename std::enable_if<isMoveConstructible<T>, void>::type
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
        typename std::enable_if<isCopyConstructible<T> && !isMoveConstructible<T>, void>::type
        Move(T* target, T* source)
    {
        new (target) T(*source);
        source->~T();
    }

    /**
    * Move an object from 'source' to 'target' using move memory copy.
    * This is only valid for POD types.
    */
    template<typename T>
    NFE_INLINE static
        typename std::enable_if<!isMoveConstructible<T> && !isCopyConstructible<T>, void>::type
        Move(T* target, T* source)
    {
        memcpy(target, source, sizeof(T));
    }
};


} // namespace Common
} // namespace NFE
