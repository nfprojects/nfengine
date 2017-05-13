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
    using DeleterFunc = typename SharedPtrData<T>::DeleterFunc;

    // Initialize with null pointer
    SharedPtr();
    SharedPtr(std::nullptr_t);

    /**
     * Initialize with a raw pointer.
     * @remarks Use with caution! MakeSharedPtr is recommended.
     */
    explicit SharedPtr(T* ptr, const DeleterFunc& deleter = SharedPtrData<T>::GetDefaultDeleter());

    SharedPtr(const SharedPtr& rhs);
    SharedPtr(SharedPtr&& rhs);
    SharedPtr(UniquePtr<T>&& rhs);
    SharedPtr& operator = (SharedPtr&& rhs);
    SharedPtr& operator = (const SharedPtr& rhs);
    ~SharedPtr();

    /**
     * Implicit conversion to related type.
     */
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
    void Reset(T* newPtr = nullptr, const DeleterFunc& deleter = SharedPtrData<T>::GetDefaultDeleter());

    /**
     * Check if pointer is not null.
     */
    operator bool() const;

    /**
     * Compare pointers.
     */
    bool operator == (const SharedPtr& other) const;
    bool operator != (const SharedPtr& other) const;
    bool operator == (const T* other) const;
    bool operator != (const T* other) const;

    /**
     * Get number of "strong" object references (number SharedPtr objects pointing to the target).
     */
    uint32 RefCount() const;

    /**
     * Get number of "weak" references (total number of SharedPtr and WeakPtr objects pointing to the target).
     */
    uint32 WeakRefCount() const;

private:
    using SharedBlock = SharedPtrData<T>;

    // pointed object
    T* mPointer;

    // shared control block
    GenericSharedPtrData* mData;

    SharedBlock* GetSharedData() const { return static_cast<SharedBlock*>(mData); }

    template<typename TargetType, typename SourceType>
    friend SharedPtr<TargetType> StaticCast(const SharedPtr<SourceType>&);

    template<typename TargetType, typename SourceType>
    friend SharedPtr<TargetType> ReinterpretCast(const SharedPtr<SourceType>&);

    template<typename U>
    friend bool operator == (const SharedPtr<U>& lhs, const WeakPtr<U>& rhs);

    template<typename U>
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
template<typename TargetType, typename SourceType>
SharedPtr<TargetType> StaticCast(const SharedPtr<SourceType>& source);

/**
 * Reinterpret cast a shared pointer.
 */
template<typename TargetType, typename SourceType>
SharedPtr<TargetType> ReinterpretCast(const SharedPtr<SourceType>& source);

} // namespace Common
} // namespace NFE


// Definitions go here:
#include "SharedPtrImpl.hpp"
