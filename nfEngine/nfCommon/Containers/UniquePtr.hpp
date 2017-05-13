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


template<typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr
{
public:
    UniquePtr();
    UniquePtr(T* ptr);
    UniquePtr(UniquePtr&& rhs);
    ~UniquePtr();
    UniquePtr& operator = (T* ptr);
    UniquePtr& operator = (UniquePtr&& ptr);

    /**
     * Access pointed object.
     */
    T** operator&();
    T* operator->() const;
    T& operator*() const;
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

private:
    T* mPointer;

    // disable copy methods
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
};


/**
 * Create unique pointer.
 */
template<typename T, typename ... Args>
UniquePtr<T> MakeUniquePtr(Args&& ... args);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "UniquePtrImpl.hpp"
