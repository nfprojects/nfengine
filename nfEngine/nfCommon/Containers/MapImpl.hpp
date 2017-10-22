/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Map data container declaration
 */

#pragma once

#include "Map.hpp"


namespace NFE {
namespace Common {


template<typename KeyType, typename ValueType, typename Comparator>
uint32 Map<KeyType, ValueType, Comparator>::Size() const
{
    return mSet.Size();
}

template<typename KeyType, typename ValueType, typename Comparator>
bool Map<KeyType, ValueType, Comparator>::Empty() const
{
    return mSet.Empty();
}

template<typename KeyType, typename ValueType, typename Comparator>
void Map<KeyType, ValueType, Comparator>::Clear()
{
    mSet.Clear();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::ConstIterator Map<KeyType, ValueType, Comparator>::Begin() const
{
    return mSet.Begin();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::Iterator Map<KeyType, ValueType, Comparator>::Begin()
{
    return mSet.Begin();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::ConstIterator Map<KeyType, ValueType, Comparator>::End() const
{
    return mSet.End();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::Iterator Map<KeyType, ValueType, Comparator>::End()
{
    return mSet.End();
}

template<typename KeyType, typename ValueType, typename Comparator>
bool typename Map<KeyType, ValueType, Comparator>::Exists(const KeyType& key) const
{
    const InternalKey internalKey(key, ValueType());
    return mSet.Exists(internalKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::ConstIterator Map<KeyType, ValueType, Comparator>::Find(const KeyType& key) const
{
    const InternalKey internalKey(key, ValueType());
    return mSet.Find(internalKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::Iterator Map<KeyType, ValueType, Comparator>::Find(const KeyType& key)
{
    const InternalKey internalKey(key, ValueType());
    return mSet.Find(internalKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::InsertResult Map<KeyType, ValueType, Comparator>::Insert(const KeyType& key, const ValueType& value)
{
    return mSet.Insert(InternalKey(key, value));
}


template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::InsertResult Map<KeyType, ValueType, Comparator>::Insert(const KeyType& key, ValueType&& value)
{
    return mSet.Insert(std::move(InternalKey(key, std::move(value))));
}

template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::InsertResult Map<KeyType, ValueType, Comparator>::InsertOrReplace(const KeyType& key, const ValueType& value)
{
    return mSet.InsertOrReplace(InternalKey(key, value));
}


template<typename KeyType, typename ValueType, typename Comparator>
typename Map<KeyType, ValueType, Comparator>::InsertResult Map<KeyType, ValueType, Comparator>::InsertOrReplace(const KeyType& key, ValueType&& value)
{
    return mSet.InsertOrReplace(std::move(InternalKey(key, std::move(value))));
}

template<typename KeyType, typename ValueType, typename Comparator>
ValueType& Map<KeyType, ValueType, Comparator>::operator[](const KeyType& key)
{
    const InternalKey tempKey(key, ValueType());
    const Iterator iter = mSet.Find(tempKey);

    NFE_ASSERT(iter != mSet.End(), "Given key does not exist in the map");

    return (*iter).second;
}

template<typename KeyType, typename ValueType, typename Comparator>
const ValueType& Map<KeyType, ValueType, Comparator>::operator[](const KeyType& key) const
{
    const InternalKey tempKey(key, ValueType());
    const ConstIterator iter = mSet.Find(tempKey);

    NFE_ASSERT(iter != mSet.End(), "Given key does not exist in the map");

    return (*iter).second;
}

template<typename KeyType, typename ValueType, typename Comparator>
bool Map<KeyType, ValueType, Comparator>::Erase(const KeyType& key)
{
    const InternalKey tempKey(key, ValueType());
    return mSet.Erase(tempKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
bool Map<KeyType, ValueType, Comparator>::Erase(const ConstIterator& iterator)
{
    return mSet.Erase(iterator);
}

template<typename KeyType, typename ValueType, typename Comparator>
bool Map<KeyType, ValueType, Comparator>::Erase(const Iterator& iterator)
{
    return mSet.Erase(iterator);
}

} // namespace Common
} // namespace NFE