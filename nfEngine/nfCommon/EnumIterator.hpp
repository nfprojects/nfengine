/**
* @file
* @author Mkkulagowski (mkkulagowski(at)gmail.com)
* @brief  EnumIterator template class definition.
*/

#pragma once


namespace NFE {
namespace Common {

template<typename T>
class EnumIterator
{
public:
    class Iterator
    {
    public:
        Iterator(int value)
            : mValue(value)
        {
        }

        T operator*(void) const
        {
            return (T)mValue;
        }

        void operator++(void)
        {
            ++mValue;
        }

        bool operator!=(const Iterator rhs) const
        {
            return mValue != rhs.mValue;
        }

        bool operator==(const Iterator rhs) const
        {
            return mValue == rhs.mValue;
        }

    private:
        int mValue;
    };

    typename EnumIterator<T>::Iterator begin() const
    {
        return typename EnumIterator<T>::Iterator((int)T::First);
    }

    typename EnumIterator<T>::Iterator end() const
    {
        return typename EnumIterator<T>::Iterator(((int)T::Last) + 1);
    }
};
  /*
template< typename T >
typename EnumIterator<T>::Iterator begin(EnumIterator<T>)
{
    return typename EnumIterator<T>::Iterator((int)T::First);
}

template< typename T >
typename EnumIterator<T>::Iterator end(EnumIterator<T>)
{
    return typename EnumIterator<T>::Iterator(((int)T::Last) + 1);
}   */

} // namespace Common
} // namespace NFE
