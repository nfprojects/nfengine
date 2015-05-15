/**
* @brief Common declarations for all the tests.
*/

#include "PCH.hpp"

/*
 * Fisher-Yates shuffle algorithm
 *
 * Select @p n random elements in a container.
 * The selected elements are placed at the beginning.
 *
 * For example, executing the function on input:
 *     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
 * with n = 4, may result in:
 *     {8, 6, 4, 7, 0, 1, 2, 3, 5, 9},
 *
 * @param begin,end Iterators pointing at the container beginning and end.
 * @param n Number to elements to select.
 */
template<class Iter>
void random_unique(Iter begin, Iter end, size_t n)
{
    size_t left = std::distance(begin, end);

    if (n > left)
        n = left;

    while (n--)
    {
        Iter r = begin;
        std::advance(r, rand() % left);
        std::swap(*begin, *r);
        ++begin;
        --left;
    }
}
