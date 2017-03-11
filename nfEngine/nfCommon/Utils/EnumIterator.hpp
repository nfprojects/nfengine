/**
 * @file
 * @author Mkkulagowski (mkkulagowski(at)gmail.com)
 * @brief  EnumIterator template class definition.
 */

#pragma once

namespace {

/**
 * SFINAE struct to assure, that used enum contains 'First' value
 */
template<typename T>
struct contains_First
{
    using yes = char[1];
    using no  = char[2];

    template<typename U> static yes& test(decltype(U::First)*);
    template<typename U> static no&  test(...);

    static bool const value = sizeof(test<T>(0)) == sizeof(yes);
};

/**
 * SFINAE struct to assure, that used enum contains 'Last' value
 */
template<typename T>
struct contains_Last
{
    using yes = char[1];
    using no  = char[2];

    template<typename U> static yes& test(decltype(U::Last)*);
    template<typename U> static no&  test(...);

    static bool const value = sizeof(test<T>(0)) == sizeof(yes);
};

/**
 * SFINAE functions to assure, that 'First' and 'Last' enum values are in the right order.
 * If enum does not contain either 'First' or 'Last' value - returns true, to hide errors.
 */
template <typename T>
std::enable_if_t<contains_First<T>::value && contains_Last<T>::value, bool>
constexpr right_FL_order()
{
    return T::First <= T::Last;
}

template <typename T>
std::enable_if_t<!(contains_First<T>::value && contains_Last<T>::value), bool>
constexpr right_FL_order()
{
    return true;
}

} // namespace

namespace NFE {
namespace Common {

/**
 * Helper class to allow iteration through enum via ranged-based for loops
 * Enums should contain First and Last elements that points to chosen values, like so:
 * enum class Enum {
 *    None,
 *    value_1,
 *    value_2,
 *    ...,
 *    value_n,
 *
 *    First = value_1,
 *    Last = value_n };
 */
template<typename T>
class EnumIterator
{
    // Asserts to assure, that T is an enumeration with First and Last elements in the right order
    static_assert(std::is_enum<T>::value,     "Type must be an enumeration");
    static_assert(::contains_First<T>::value, "Enumeration must contain an element called First");
    static_assert(::contains_Last<T>::value,  "Enumeration must contain an element called Last");
    static_assert(::right_FL_order<T>(),      "'First' element cannot be greater then the 'Last'");

public:
    class Iterator
    {
    public:
        Iterator(int value)
            : mValue(value)
        {
        }

        T operator*() const
        {
            return static_cast<T>(mValue);
        }

        void operator++()
        {
            ++mValue;
        }

        bool operator!=(const Iterator& rhs) const
        {
            return mValue != rhs.mValue;
        }

        bool operator==(const Iterator& rhs) const
        {
            return mValue == rhs.mValue;
        }

    private:
        int mValue;
    };

    using TemplateEnumIterator = typename EnumIterator<T>::Iterator;

    TemplateEnumIterator begin() const
    {
        return TemplateEnumIterator(static_cast<int>(T::First));
    }

    TemplateEnumIterator end() const
    {
        return TemplateEnumIterator((static_cast<int>(T::Last)) + 1);
    }
};

} // namespace Common
} // namespace NFE
