/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of C++ language specific constructions.
 */

#pragma once

#include "nfCommon.hpp"


// use this inside a class declaration to make it non-copyable
#define NFE_MAKE_NONCOPYABLE(Type)              \
private:                                        \
    Type(const Type&) = delete;                 \
    Type& operator=(const Type&) = delete;


// use this inside a class declaration to make it non-moveable
#define NFE_MAKE_NONMOVEABLE(Type)              \
private:                                        \
    Type(Type&&) = delete;                      \
    Type& operator=(Type&&) = delete;


// macro for disabling "unsused parameter"
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif // UNUSED
