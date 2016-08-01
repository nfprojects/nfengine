/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of C++ language specific constructions.
 */

#pragma once

#include "nfCommon.hpp"


// inherit from this class to make other non-copyable
class NFCOMMON_API NonCopyable
{
public:
    NonCopyable() = default;
    virtual ~NonCopyable() = default;

private:
    NonCopyable(NonCopyable const &) = delete;
    void operator=(NonCopyable const &) = delete;
};


// macro for disabling "unsused parameter"
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif // UNUSED
