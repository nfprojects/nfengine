/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  HashMap data container declaration
 */

#pragma once

#include "../nfCommon.hpp"
#include "HashSet.hpp"

#include <utility>


namespace NFE {
namespace Common {


template<typename KeyType, typename ValueType, typename HashPolicy = DefaultHashPolicy<KeyType>>
class HashMap final
{
public:

    struct InternalKey
    {
        KeyType first;
        ValueType second;

        InternalKey() = default;
        InternalKey(const InternalKey&) = default;
        InternalKey(InternalKey&&) = default;
        InternalKey& operator = (const InternalKey&) = default;
        InternalKey& operator = (InternalKey&&) = default;
        NFE_INLINE InternalKey(const KeyType& key, const ValueType& value) : first(key), second(value) { }
        NFE_INLINE InternalKey(KeyType&& key, ValueType&& value) : first(std::move(key)), second(std::move(value)) { }

        NFE_INLINE bool operator == (const InternalKey& rhs) const
        {
            return first == rhs.first;
        }
    };

    // hash policy that takes first pair element into account
    struct InternalHashPolicy
    {
        uint32 operator() (const InternalKey& in) const
        {
            return GetHash(in.first);
        }
    };

    using InternalSet = HashSet<InternalKey, InternalHashPolicy>;
    using InsertResult = typename InternalSet::InsertResult;
    using ConstIterator = typename InternalSet::ConstIterator;
    using Iterator = typename InternalSet::Iterator;

    NFE_INLINE uint32 Size() const;
    NFE_INLINE bool Empty() const;
    NFE_INLINE void Clear();

    NFE_INLINE ConstIterator Begin() const;
    NFE_INLINE Iterator Begin();

    NFE_INLINE ConstIterator End() const;
    NFE_INLINE Iterator End();

    NFE_INLINE ConstIterator Find(const KeyType& key) const;
    NFE_INLINE Iterator Find(const KeyType& key);

    NFE_INLINE InsertResult Insert(const KeyType& key, const ValueType& value);
    NFE_INLINE InsertResult Insert(const KeyType& key, ValueType&& value);

    NFE_INLINE InsertResult InsertOrReplace(const KeyType& key, const ValueType& value);
    NFE_INLINE InsertResult InsertOrReplace(const KeyType& key, ValueType&& value);

    ValueType& operator[](const KeyType& key);
    const ValueType& operator[](const KeyType& key) const;

    NFE_INLINE bool Erase(const KeyType& key);
    NFE_INLINE bool Erase(const ConstIterator& iterator);
    NFE_INLINE bool Erase(const Iterator& iterator);

    NFE_INLINE ConstIterator cbegin() const { return Begin(); }
    NFE_INLINE ConstIterator cend() const { return Begin(); }
    NFE_INLINE ConstIterator begin() const { return Begin(); }
    NFE_INLINE ConstIterator end() const { return End(); }
    NFE_INLINE Iterator begin() { return Begin(); }
    NFE_INLINE Iterator end() { return End(); }

private:

    // internal hash set
    InternalSet mSet;
};


} // namespace Common
} // namespace NFE


#include "HashMapImpl.hpp"
