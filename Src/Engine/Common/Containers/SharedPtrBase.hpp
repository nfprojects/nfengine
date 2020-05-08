/**
 * @file
 * @author  Witek902
 * @brief   Shared pointer declaration
 */

#pragma once

#include "UniquePtr.hpp"
#include "SharedPtrData.hpp"


namespace NFE {
namespace Common {


template<typename T>
class WeakPtr;

class SharedPtrBase
{
public:
    NFE_FORCE_INLINE SharedPtrBase() = default;

    NFE_FORCE_INLINE explicit SharedPtrBase(SharedPtrData* data)
        : mData(data)
    { }

    // Get number of "strong" object references (number SharedPtr objects pointing to the target).
    NFCOMMON_API uint32 RefCount() const;

    // Get number of "weak" references (total number of SharedPtr and WeakPtr objects pointing to the target).
    NFCOMMON_API uint32 WeakRefCount() const;

protected:
    // shared control block
    SharedPtrData* mData = nullptr;
};

/**
 * Common base for SharedPtr and WeakPtr.
 */
template<typename T>
class SharedPtrTypedBase : public SharedPtrBase
{
public:
    NFE_FORCE_INLINE SharedPtrTypedBase() = default;

    NFE_FORCE_INLINE explicit SharedPtrTypedBase(T* pointer, SharedPtrData* data)
        : SharedPtrBase(data)
        , mPointer(pointer)
    { }

protected:

    // Pointed object (for fast access).
    // Note that this pointer may be different than mData->GetPointer(), for example after casting to parent
    // class type.
    T* mPointer = nullptr;
};


} // namespace Common
} // namespace NFE
