/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Weak pointer declaration
 */

#pragma once

#include "SharedPtrBase.hpp"


namespace NFE {
namespace Common {


template<typename T>
class WeakPtr final : public SharedPtrBase<T>
{
public:
    using DeleterFunc = typename SharedPtrData<T>::DeleterFunc;
    using SharedBlockType = SharedPtrData<T>;

    NFE_FORCE_INLINE WeakPtr() = default;
    NFE_FORCE_INLINE WeakPtr(std::nullptr_t) : WeakPtr() { }

    WeakPtr(const SharedPtr<T>& rhs);
    WeakPtr(const WeakPtr& rhs);
    WeakPtr(WeakPtr&& rhs);

    WeakPtr& operator = (const SharedPtr<T>& rhs);
    WeakPtr& operator = (const WeakPtr& rhs);
    WeakPtr& operator = (WeakPtr&& rhs);

    /**
     * Implicit conversion constructor.
     */
    template<typename SourceType>
    WeakPtr(const WeakPtr<SourceType>& rhs);

    ~WeakPtr();

    /**
     * Check if the pointed object is still valid.
     */
    bool Valid() const;

    /**
     * Turn the weak pointer into a shared (strong) pointer.
     */
    SharedPtr<T> Lock() const;

    /**
     * Release the ownership.
     */
    void Reset();

    /**
     * Compare pointers.
     */
    bool operator == (const SharedPtr<T>& other) const;
    bool operator == (const WeakPtr<T>& other) const;

private:
    template<typename TargetType, typename SourceType>
    friend WeakPtr<TargetType> Cast(const WeakPtr<SourceType>&);

    template<typename U>
    friend bool operator == (const SharedPtr<U>& lhs, const WeakPtr<U>& rhs);

    template<typename U>
    friend class WeakPtr;
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

/**
 * Calculate hash of a weak pointer.
 */
template<typename T>
NFE_FORCE_INLINE uint32 GetHash(const WeakPtr<T>& x);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "WeakPtrImpl.hpp"
