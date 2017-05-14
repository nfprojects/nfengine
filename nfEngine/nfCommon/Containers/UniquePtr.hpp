/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unique pointer declaration
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {


template<typename T>
struct DefaultDeleter
{
    static void Delete(T* pointer)
    {
        delete pointer;
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
    UniquePtrBase();
    UniquePtrBase(T* ptr);
    UniquePtrBase(UniquePtrBase&& rhs);
    ~UniquePtrBase();
    UniquePtrBase& operator = (T* ptr);
    UniquePtrBase& operator = (UniquePtrBase&& ptr);

    /**
     * Access pointed object.
     */
    T** operator&();
    T* Get() const;

    /**
     * Set a new object.
     */
    void Reset(T* newPtr = nullptr);

    /**
     * Pass ownership to the callee.
     * @remarks Use with caution.
     */
    T* Release();

    /**
     * Check if pointer is not null.
     */
    operator bool() const;

    /**
     * Compare with raw pointer.
     */
    bool operator == (const T* other) const;
    bool operator != (const T* other) const;

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
    UniquePtr() {}
    UniquePtr(nullptr_t) : UniquePtr() {}
    UniquePtr(T* ptr) : UniquePtrBase<T, Deleter>(ptr) {}
    UniquePtr(UniquePtr&& rhs) : UniquePtrBase<T, Deleter>(std::move(rhs)) {}
    UniquePtr& operator = (T* ptr) { UniquePtrBase<T, Deleter>::operator=(ptr); return *this; }
    UniquePtr& operator = (UniquePtr&& ptr) { UniquePtrBase<T, Deleter>::operator=(std::move(ptr)); return *this; }

    /**
     * Convert to another type (e.g. base class).
     */
    template<typename U>
    operator UniquePtr<U>();

    /**
     * Access pointed object.
     */
    T* operator->() const;
    T& operator*() const;
};

/**
 * UniquePtr - array of objects.
 */
template<typename T, typename Deleter>
class UniquePtr<T[], Deleter>
    : public UniquePtrBase<T, Deleter>
{
public:
    UniquePtr() {}
    UniquePtr(nullptr_t) : UniquePtr() {}
    UniquePtr(T* ptr) : UniquePtrBase<T, Deleter>(ptr) {}
    UniquePtr(UniquePtr&& rhs) : UniquePtrBase<T, Deleter>(std::move(rhs)) {}
    UniquePtr& operator = (T* ptr) { UniquePtrBase<T, Deleter>::operator=(ptr); return *this; }
    UniquePtr& operator = (UniquePtr&& ptr) { UniquePtrBase<T, Deleter>::operator=(std::move(ptr)); return *this; }

    /**
     * Convert to another type (e.g. base class).
     */
    template<typename U>
    operator UniquePtr<U[]>();

    /**
     * Access array element.
     */
    T& operator[] (size_t i) const;
};


/**
 * Create unique pointer.
 */
template<typename T, typename ... Args>
UniquePtr<T> MakeUniquePtr(Args&& ... args);


/**
 * Static cast a unique pointer.
 */
template<typename T, typename U>
UniquePtr<T> StaticCast(UniquePtr<U>&& source);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "UniquePtrImpl.hpp"
