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
class WeakPtr;

template<typename T>
class SharedPtr
{
public:
    SharedPtr();
    SharedPtr(nullptr_t);
    SharedPtr(T* ptr);
    SharedPtr(const SharedPtr& rhs);
    SharedPtr(SharedPtr&& rhs);
    ~SharedPtr();
    SharedPtr& operator = (const SharedPtr& rhs);
    SharedPtr& operator = (SharedPtr&& rhs);

    template<typename U>
    operator SharedPtr<U>() const
    {
        static_assert(std::is_base_of<U, T>::value, "Cannot downcast shared pointer");
        return StaticCast<U>(*this);
    }

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
     * Compare pointers.
     */
    bool operator == (const SharedPtr& other) const;
    bool operator == (const T* other) const;
    bool operator != (const T* other) const;

    /**
     * Get number of object references (number SharedPtr objects pointing to the target).
     */
    uint32 RefCount() const;

private:
    using SharedBlock = SharedPtrData<T>;

    // pointed object
    T* mPointer;

    // shared control block
    SharedBlock* mData;

    template<typename T, typename U>
    friend SharedPtr<T> StaticCast(const SharedPtr<U>&);

    template<typename T>
    friend bool operator == (const SharedPtr<T>& lhs, const WeakPtr<T>& rhs);

    template<typename T>
    friend class WeakPtr;
};


/**
 * Create shared pointer.
 */
template<typename T, typename ... Args>
SharedPtr<T> MakeSharedPtr(Args&& ... args);

/**
 * Static cast a shared pointer.
 */
template<typename T, typename U>
SharedPtr<T> StaticCast(const SharedPtr<U>& source);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "SharedPtrImpl.hpp"
