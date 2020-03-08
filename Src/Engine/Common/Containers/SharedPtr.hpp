/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Shared pointer declaration
 */

#pragma once

#include "UniquePtr.hpp"
#include "SharedPtrBase.hpp"
#include "Hash.hpp"


namespace NFE {
namespace Common {


template<typename T>
class WeakPtr;

template<typename T>
class SharedPtr final : public SharedPtrBase<T>
{
public:
    using DeleterFunc = typename SharedPtrData<T>::DeleterFunc;
    using SharedBlockType = SharedPtrData<T>;

    // Initialize with null pointer
    SharedPtr() { }
    SharedPtr(std::nullptr_t) : SharedPtr() { }

    /**
     * Initialize with a raw pointer.
     * @remarks Use with caution! MakeSharedPtr is recommended.
     */
    explicit SharedPtr(T* ptr, const DeleterFunc& deleter = SharedPtrData<T>::GetDefaultDeleter());

    SharedPtr(const SharedPtr& rhs);
    SharedPtr(SharedPtr&& rhs);
    SharedPtr(UniquePtr<T>&& rhs);
    SharedPtr& operator = (const SharedPtr& rhs);
    SharedPtr& operator = (SharedPtr&& rhs);
    SharedPtr& operator = (UniquePtr<T>&& rhs);
    ~SharedPtr();

    /**
     * Implicit conversion constructor.
     */
    template<typename SourceType>
    SharedPtr(const SharedPtr<SourceType>& rhs);

    /**
     * Implicit conversion assignment.
     */
    template<typename SourceType>
    SharedPtr& operator = (const SharedPtr<SourceType>& rhs);

    /**
     * Access pointed object.
     */
    T* operator->() const;
    T& operator*() const;
    T* Get() const;

    /**
     * Set a new object.
     */
    void Reset(T* newPtr = nullptr, const DeleterFunc& deleter = SharedPtrData<T>::GetDefaultDeleter());

    /**
     * Check if pointer is not null.
     */
    operator bool() const;

    /**
     * Compare pointers.
     */
    bool operator == (std::nullptr_t) const;
    bool operator != (std::nullptr_t) const;
    bool operator == (const SharedPtr& other) const;
    bool operator != (const SharedPtr& other) const;
    bool operator == (const T* other) const;
    bool operator != (const T* other) const;

private:
    template<typename TargetType, typename SourceType>
    friend SharedPtr<TargetType> StaticCast(const SharedPtr<SourceType>&);

    template<typename TargetType, typename SourceType>
    friend SharedPtr<TargetType> DynamicCast(const SharedPtr<SourceType>&);

    template<typename U>
    friend bool operator == (const SharedPtr<U>& lhs, const WeakPtr<U>& rhs);

    template<typename U>
    friend class SharedPtr;

    template<typename U>
    friend class WeakPtr;
};


/**
 * Create shared pointer.
 */
template<typename T, typename ... Args>
NFE_INLINE SharedPtr<T> MakeSharedPtr(Args&& ... args);

/**
 * Static cast a shared pointer.
 */
template<typename TargetType, typename SourceType>
SharedPtr<TargetType> StaticCast(const SharedPtr<SourceType>& source);

/**
 * Dynamic cast a shared pointer.
 */
// TODO this should be removed and replaced with RTTI's Cast
template<typename TargetType, typename SourceType>
SharedPtr<TargetType> DynamicCast(const SharedPtr<SourceType>& source);

/**
 * Calculate hash of shared pointer.
 */
template<typename T>
NFE_INLINE uint32 GetHash(const SharedPtr<T>& x);


} // namespace Common
} // namespace NFE


// Definitions go here:
#include "SharedPtrImpl.hpp"
