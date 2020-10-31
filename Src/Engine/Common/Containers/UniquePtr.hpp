/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unique pointer declaration
 */

#pragma once

#include "Hash.hpp"
#include "../Memory/DefaultAllocator.hpp"


namespace NFE {
namespace Common {


template<typename T>
struct DefaultDeleter
{
    static void Delete(T* pointer)
    {
        if (pointer)
        {
            pointer->~T();
            NFE_FREE(pointer);
        }
    }
};

template<typename T>
struct DefaultDeleter<T[]>
{
    static void Delete(T* pointer)
    {
        delete[] pointer;
    }
};

template<typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr;


template<typename T, typename Deleter>
class UniquePtrBase
{
public:
    NFE_FORCE_INLINE UniquePtrBase();
    NFE_FORCE_INLINE UniquePtrBase(T* ptr);
    NFE_FORCE_INLINE UniquePtrBase(UniquePtrBase&& rhs);
    NFE_FORCE_INLINE ~UniquePtrBase();

    UniquePtrBase& operator = (T* ptr);
    UniquePtrBase& operator = (UniquePtrBase&& ptr);

    /**
     * Access pointed object.
     */
    NFE_FORCE_INLINE T* Get() const;

    /**
     * Get pointer to internal pointer.
     */
    NFE_FORCE_INLINE T** GetPtr();

    /**
     * Set a new object.
     */
    void Reset(T* newPtr = nullptr);

    /**
     * Pass ownership to the callee.
     * @remarks Use with caution.
     */
    NFE_FORCE_INLINE [[nodiscard]] T* ReleaseOwnership();

    /**
     * Check if pointer is not null.
     */
    NFE_FORCE_INLINE operator bool() const;

    /**
     * Compare with raw pointer.
     */
    NFE_FORCE_INLINE bool operator == (const T* other) const;
    NFE_FORCE_INLINE bool operator != (const T* other) const;

    /**
     * Compare with other unique pointer.
     */
    bool operator == (const UniquePtrBase& other) const;
    bool operator != (const UniquePtrBase& other) const;

protected:
    T* mPointer;

private:
    // disable copy methods
    UniquePtrBase(const UniquePtrBase&) = delete;
    UniquePtrBase& operator=(const UniquePtrBase&) = delete;
};

/**
 * Unique pointer - single object.
 */
template<typename T, typename Deleter>
class UniquePtr : public UniquePtrBase<T, Deleter>
{
public:
    NFE_FORCE_INLINE UniquePtr() = default;
    NFE_FORCE_INLINE UniquePtr(std::nullptr_t) : UniquePtr() {}
    NFE_FORCE_INLINE explicit UniquePtr(T* ptr) : UniquePtrBase<T, Deleter>(ptr) {}
    NFE_FORCE_INLINE UniquePtr(UniquePtr&& rhs) : UniquePtrBase<T, Deleter>(std::move(rhs)) {}
    NFE_FORCE_INLINE UniquePtr& operator = (T* ptr) { UniquePtrBase<T, Deleter>::operator=(ptr); return *this; }
    NFE_FORCE_INLINE UniquePtr& operator = (UniquePtr&& ptr) { UniquePtrBase<T, Deleter>::operator=(std::move(ptr)); return *this; }

    /**
     * Convert to another type (e.g. base class).
     */
    template<typename U>
    NFE_FORCE_INLINE operator UniquePtr<U>();

    /**
     * Access pointed object.
     */
    NFE_FORCE_INLINE T* operator->() const;
    NFE_FORCE_INLINE T& operator*() const;
};

/**
 * UniquePtr - array of objects.
 */
template<typename T, typename Deleter>
class UniquePtr<T[], Deleter>
    : public UniquePtrBase<T, Deleter>
{
public:
    NFE_FORCE_INLINE UniquePtr() {}
    NFE_FORCE_INLINE UniquePtr(std::nullptr_t) : UniquePtr() {}
    NFE_FORCE_INLINE explicit UniquePtr(T* ptr) : UniquePtrBase<T, Deleter>(ptr) {}
    NFE_FORCE_INLINE UniquePtr(UniquePtr&& rhs) : UniquePtrBase<T, Deleter>(std::move(rhs)) {}
    NFE_FORCE_INLINE UniquePtr& operator = (T* ptr) { UniquePtrBase<T, Deleter>::operator=(ptr); return *this; }
    NFE_FORCE_INLINE UniquePtr& operator = (UniquePtr&& ptr) { UniquePtrBase<T, Deleter>::operator=(std::move(ptr)); return *this; }

    /**
     * Convert to another type (e.g. base class).
     */
    template<typename U>
    NFE_FORCE_INLINE operator UniquePtr<U[]>();

    /**
     * Access array element.
     */
    NFE_FORCE_INLINE T& operator[] (size_t i) const;
};


namespace detail {
    template<typename>
    constexpr bool is_unbounded_array_v = false;
    template<typename T>
    constexpr bool is_unbounded_array_v<T[]> = true;

    template<typename>
    constexpr bool is_bounded_array_v = false;
    template<typename T, size_t N>
    constexpr bool is_bounded_array_v<T[N]> = true;
} // namespace detail


/**
 * Create unique pointer.
 */

template<typename T, typename Deleter, typename... Args>
NFE_INLINE std::enable_if_t<!std::is_array<T>::value, UniquePtr<T,Deleter>> MakeUniquePtr(Args&& ... args);

template<typename T, typename... Args>
NFE_INLINE std::enable_if_t<!std::is_array<T>::value, UniquePtr<T>> MakeUniquePtr(Args&& ... args);

template<typename T, typename Deleter = DefaultDeleter<T>>
NFE_INLINE std::enable_if_t<detail::is_unbounded_array_v<T>, UniquePtr<T,Deleter>> MakeUniquePtr(size_t n);

template<typename T, typename Deleter = DefaultDeleter<T>, typename... Args>
NFE_INLINE std::enable_if_t<detail::is_bounded_array_v<T>, UniquePtr<T,Deleter>> MakeUniquePtr(Args&& ...) = delete;

/**
 * Static cast a unique pointer.
 */
template<typename T, typename U>
NFE_INLINE UniquePtr<T> StaticCast(UniquePtr<U>&& source);

/**
 * Additional compare
 */
template<typename T, typename Deleter>
bool operator == (const T* lhs, const UniquePtr<T, Deleter>& rhs)
{
    return rhs == lhs;
}

/**
 * Additional compare
 */
template<typename T, typename Deleter>
bool operator != (const T* lhs, const UniquePtr<T, Deleter>& rhs)
{
    return rhs != lhs;
}

/**
 * Calculate hash of shared pointer.
 */
template<typename T>
NFE_INLINE uint32 GetHash(const UniquePtr<T>& x);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "UniquePtrImpl.hpp"
