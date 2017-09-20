/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Map data container declaration
 */

#pragma once

#include "../nfCommon.hpp"
#include "Set.hpp"

#include <utility>


namespace NFE {
namespace Common {


template<typename KeyType, typename ValueType, typename Comparator = DefaultComparator<KeyType>>
class Map final
{
public:

    using InternalKey = std::pair<KeyType, ValueType>;

    // comparator that compares only first element of the internal key
    struct InternalComparator
    {
        bool Less(const InternalKey& left, const InternalKey& right) const
        {
            const Comparator comparator;
            return comparator.Less(left.first, right.first);
        }

        bool Equal(const InternalKey& left, const InternalKey& right) const
        {
            const Comparator comparator;
            return comparator.Equal(left.first, right.first);
        }
    };

    using InternalSet = Set<InternalKey, InternalComparator>;
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

    // internal set
    Set<InternalKey, InternalComparator> mSet;
};


} // namespace Common
} // namespace NFE


#include "MapImpl.hpp"
