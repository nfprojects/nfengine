/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  HashMap data container definitions
 */

#pragma once

#include "HashMap.hpp"


namespace NFE {
namespace Common {


template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
uint32 HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Size() const
{
    return mSet.Size();
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
bool HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Empty() const
{
    return mSet.Empty();
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
void HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Clear()
{
    mSet.Clear();
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::ConstIterator HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Begin() const
{
    return mSet.Begin();
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Iterator HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Begin()
{
    return mSet.Begin();
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::ConstIterator HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::End() const
{
    return mSet.End();
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Iterator HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::End()
{
    return mSet.End();
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
bool HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Exists(const KeyType& key) const
{
    const InternalKey internalKey(key, ValueType());
    return mSet.Exists(internalKey);
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::ConstIterator HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Find(const KeyType& key) const
{
    const InternalKey internalKey(key);
    return mSet.Find(internalKey);
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Iterator HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Find(const KeyType& key)
{
    const InternalKey internalKey(key);
    return mSet.Find(internalKey);
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::InsertResult HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Insert(const KeyType& key, const ValueType& value)
{
    return mSet.Insert(InternalKey(key, value));
}


template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::InsertResult HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Insert(const KeyType& key, ValueType&& value)
{
    return mSet.Insert(std::move(InternalKey(key, std::move(value))));
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::InsertResult HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::InsertOrReplace(const KeyType& key, const ValueType& value)
{
    return mSet.InsertOrReplace(InternalKey(key, value));
}


template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
typename HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::InsertResult HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::InsertOrReplace(const KeyType& key, ValueType&& value)
{
    return mSet.InsertOrReplace(std::move(InternalKey(key, std::move(value))));
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
ValueType& HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::operator[](const KeyType& key)
{
    const InternalKey tempKey(key);
    const Iterator iter = mSet.Find(tempKey);

    NFE_ASSERT(iter != mSet.End(), "Given key does not exist in the map");

    return (*iter).second;
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
const ValueType& HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::operator[](const KeyType& key) const
{
    const InternalKey tempKey(key);
    const ConstIterator iter = mSet.Find(tempKey);

    NFE_ASSERT(iter != mSet.End(), "Given key does not exist in the map");

    return (*iter).second;
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
bool HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Erase(const KeyType& key)
{
    const InternalKey tempKey(key);
    return mSet.Erase(tempKey);
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
bool HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Erase(const ConstIterator& iterator)
{
    return mSet.Erase(iterator);
}

template<typename KeyType, typename ValueType, typename HashPolicy, typename EqualsPolicy>
bool HashMap<KeyType, ValueType, HashPolicy, EqualsPolicy>::Erase(const Iterator& iterator)
{
    return mSet.Erase(iterator);
}

} // namespace Common
} // namespace NFE