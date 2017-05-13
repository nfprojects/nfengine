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
class WeakPtr final
{
public:
    WeakPtr() : mData(nullptr) { }
    WeakPtr(std::nullptr_t) : WeakPtr() { }

    WeakPtr(const SharedPtr<T>& rhs);
    WeakPtr(const WeakPtr& rhs);
    WeakPtr(WeakPtr&& rhs);

    WeakPtr& operator = (const SharedPtr<T>& rhs);
    WeakPtr& operator = (const WeakPtr& rhs);
    WeakPtr& operator = (WeakPtr&& rhs);

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
    GenericSharedPtrData* mData;

    SharedBlock* GetSharedData() const { return static_cast<SharedBlock*>(mData); }

    void Reset();

    template<typename TargetType, typename SourceType>
    friend WeakPtr<TargetType> Cast(const WeakPtr<SourceType>&);

    template<typename U>
    friend bool operator == (const SharedPtr<U>& lhs, const WeakPtr<U>& rhs);
};


/**
 * Compare shared pointer with weak pointer.
 */
template<typename T>
bool operator == (const SharedPtr<T>& lhs, const WeakPtr<T>& rhs);

/**
 * Cast a weak pointer.
 */
template<typename TargetType, typename SourceType>
SharedPtr<TargetType> Cast(const SharedPtr<SourceType>& source);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "WeakPtrImpl.hpp"
