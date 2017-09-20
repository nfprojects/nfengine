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
ConstIterator Map<KeyType, ValueType, Comparator>::Begin() const
{
    return mSet.Begin();
}

template<typename KeyType, typename ValueType, typename Comparator>
Iterator Map<KeyType, ValueType, Comparator>::Begin()
{
    return mSet.Begin();
}

template<typename KeyType, typename ValueType, typename Comparator>
ConstIterator Map<KeyType, ValueType, Comparator>::End() const
{
    return mSet.End();
}

template<typename KeyType, typename ValueType, typename Comparator>
Iterator Map<KeyType, ValueType, Comparator>::End()
{
    return mSet.End();
}

template<typename KeyType, typename ValueType, typename Comparator>
ConstIterator Map<KeyType, ValueType, Comparator>::Find(const KeyType& key) const
{
    const InternalKey internalKey(key, ValueType());
    return mSet.Find(internalKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
Iterator Map<KeyType, ValueType, Comparator>::Find(const KeyType& key)
{
    const InternalKey internalKey(key, ValueType());
    return mSet.Find(internalKey);
}

template<typename KeyType, typename ValueType, typename Comparator>
InsertResult Map<KeyType, ValueType, Comparator>::Insert(const KeyType& key, const ValueType& value)
{
    return mSet.Insert(InternalKey(key, value));
}


template<typename KeyType, typename ValueType, typename Comparator>
InsertResult Map<KeyType, ValueType, Comparator>::Insert(KeyType&& key);

template<typename KeyType, typename ValueType, typename Comparator>
ValueType& Map<KeyType, ValueType, Comparator>::operator[](const KeyType& key)
{

}

template<typename KeyType, typename ValueType, typename Comparator>
ValueType& Map<KeyType, ValueType, Comparator>::operator[](KeyType&& key)
{

}

template<typename KeyType, typename ValueType, typename Comparator>
bool Map<KeyType, ValueType, Comparator>::Erase(const KeyType& key)
{

}

template<typename KeyType, typename ValueType, typename Comparator>
bool Map<KeyType, ValueType, Comparator>::Erase(const ConstIterator& iterator)
{

}

template<typename KeyType, typename ValueType, typename Comparator>
bool Map<KeyType, ValueType, Comparator>::Erase(const Iterator& iterator)
{

}

} // namespace Common
} // namespace NFE