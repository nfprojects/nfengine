/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Shared pointer declaration
 */

#pragma once

#include "UniquePtr.hpp"
#include "SharedPtrData.hpp"


namespace NFE {
namespace Common {


template<typename T>
class SharedPtr
{
public:
    SharedPtr();
    SharedPtr(T* ptr);
    SharedPtr(const SharedPtr& rhs);
    SharedPtr(SharedPtr&& rhs);
    ~SharedPtr();
    SharedPtr& operator = (const SharedPtr& rhs);
    SharedPtr& operator = (SharedPtr&& rhs);

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
     * Check if pointer is not null.
     */
    operator bool() const;

    /**
     * Compare with raw pointer.
     */
    bool operator == (const T* other) const;
    bool operator != (const T* other) const;

    /**
     * Get number of object references (number SharedPtr objects pointing to the target).
     */
    uint32 RefCount() const;

private:
    // pointed object
    T* mPointer;

    // shared control block
    SharedPtrData* mData;
};


/**
 * Create shared pointer.
 */
template<typename T, typename ... Args>
SharedPtr<T> MakeSharedPtr(Args&& ... args);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "SharedPtrImpl.hpp"
