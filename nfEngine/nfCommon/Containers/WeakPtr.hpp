/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Weak pointer declaration
 */

#pragma once

#include "WeakPtr.hpp"


namespace NFE {
namespace Common {


template<typename T>
class WeakPtr
{
public:
    WeakPtr();
    WeakPtr(nullptr_t);

    WeakPtr(const SharedPtr<T>& rhs);
    WeakPtr(const WeakPtr<T>& rhs);
    WeakPtr(WeakPtr<T>&& rhs);

    WeakPtr& operator = (const SharedPtr<T>& rhs);
    WeakPtr& operator = (const WeakPtr<T>& rhs);
    WeakPtr& operator = (WeakPtr<T>&& rhs);

    ~WeakPtr();

    /**
     * Check if the pointed object is still valid.
     */
    bool Valid() const;

    /**
     * Turn the weak pointer into a shared pointer.
     */
    SharedPtr<T> Lock() const;

    /**
     * Get number of object references.
     */
    uint32 RefCount() const;

    /**
     * Compare pointers.
     */
    bool operator == (const SharedPtr<T>& other) const;
    bool operator == (const WeakPtr<T>& other) const;

private:
    using SharedBlock = SharedPtrData<T>;

    // shared control block
    SharedBlock* mData;

    void Reset();

    template<typename T>
    friend bool operator == (const SharedPtr<T>& lhs, const WeakPtr<T>& rhs);
};


/**
 * Compare shared pointer with weak pointer.
 */
template<typename T>
bool operator == (const SharedPtr<T>& lhs, const WeakPtr<T>& rhs);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "WeakPtrImpl.hpp"
