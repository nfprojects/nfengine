/**
 * @file   Aligned.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of memory aligning class.
 */

#include "stdafx.hpp"
#include "Aligned.hpp"

namespace NFE {
namespace Util {

Aligned::~Aligned()
{

}

void* Aligned::operator new(size_t size)
{
    return _aligned_malloc(size, 16);
}

void Aligned::operator delete(void* ptr)
{
    _aligned_free(ptr);
}

void* Aligned::operator new[] (size_t size)
{
    return _aligned_malloc(size, 16);
}

void Aligned::operator delete[] (void* ptr)
{
    _aligned_free(ptr);
}

} // namespace Util
} // namespace NFE
