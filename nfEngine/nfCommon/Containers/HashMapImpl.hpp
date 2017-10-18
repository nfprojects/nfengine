/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  HashMap data container definitions
 */

#pragma once

#include "HashMap.hpp"


namespace NFE {
namespace Common {


template<typename KeyType, typename ValueType, typename Comparator>
uint32 HashMap<KeyType, ValueType, Comparator>::Size() const
{
    return mSet.Size();
}

template<typename KeyType, typename ValueType, typename Comparator>
bool HashMap<KeyType, ValueType, Comparator>::Empty() const
{
    return mSet.Empty();
}

template<typename KeyType, typename ValueType, typename Comparator>
void HashMap<KeyType, ValueType, Comparator>::Clear()
{
    mSet.Clear();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::ConstIterator HashMap<KeyType, ValueType, Comparator>::Begin() const
{
    return mSet.Begin();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::Iterator HashMap<KeyType, ValueType, Comparator>::Begin()
{
    return mSet.Begin();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::ConstIterator HashMap<KeyType, ValueType, Comparator>::End() const
{
    return mSet.End();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::Iterator HashMap<KeyType, ValueType, Comparator>::End()
{
    return mSet.End();
}

template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::ConstIterator HashMap<KeyType, ValueType, Comparator>::Find(const KeyType& key) const
{
    const InternalKey internalKey(key);
    return mSet.Find(internalKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::Iterator HashMap<KeyType, ValueType, Comparator>::Find(const KeyType& key)
{
    const InternalKey internalKey(key);
    return mSet.Find(internalKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::InsertResult HashMap<KeyType, ValueType, Comparator>::Insert(const KeyType& key, const ValueType& value)
{
    return mSet.Insert(InternalKey(key, value));
}


template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::InsertResult HashMap<KeyType, ValueType, Comparator>::Insert(const KeyType& key, ValueType&& value)
{
    return mSet.Insert(std::move(InternalKey(key, std::move(value))));
}

template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::InsertResult HashMap<KeyType, ValueType, Comparator>::InsertOrReplace(const KeyType& key, const ValueType& value)
{
    return mSet.InsertOrReplace(InternalKey(key, value));
}


template<typename KeyType, typename ValueType, typename Comparator>
typename HashMap<KeyType, ValueType, Comparator>::InsertResult HashMap<KeyType, ValueType, Comparator>::InsertOrReplace(const KeyType& key, ValueType&& value)
{
    return mSet.InsertOrReplace(std::move(InternalKey(key, std::move(value))));
}

template<typename KeyType, typename ValueType, typename Comparator>
ValueType& HashMap<KeyType, ValueType, Comparator>::operator[](const KeyType& key)
{
    const InternalKey tempKey(key);
    const Iterator iter = mSet.Find(tempKey);

    NFE_ASSERT(iter != mSet.End(), "Given key does not exist in the map");

    return (*iter).second;
}

template<typename KeyType, typename ValueType, typename Comparator>
const ValueType& HashMap<KeyType, ValueType, Comparator>::operator[](const KeyType& key) const
{
    const InternalKey tempKey(key);
    const ConstIterator iter = mSet.Find(tempKey);

    NFE_ASSERT(iter != mSet.End(), "Given key does not exist in the map");

    return (*iter).second;
}

template<typename KeyType, typename ValueType, typename Comparator>
bool HashMap<KeyType, ValueType, Comparator>::Erase(const KeyType& key)
{
    const InternalKey tempKey(key);
    return mSet.Erase(tempKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
bool HashMap<KeyType, ValueType, Comparator>::Erase(const ConstIterator& iterator)
{
    return mSet.Erase(iterator);
}

template<typename KeyType, typename ValueType, typename Comparator>
bool HashMap<KeyType, ValueType, Comparator>::Erase(const Iterator& iterator)
{
    return mSet.Erase(iterator);
}

} // namespace Common
} // namespace NFE