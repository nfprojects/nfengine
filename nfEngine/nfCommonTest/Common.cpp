/**
* @brief Definitions of common functions and classes.
*/

#include "stdafx.hpp"
#include "Common.hpp"

Latch::Latch()
    : mSet(false)
{
}

void Latch::Set()
{
    Lock lock(mMutex);
    mSet = true;
    mCV.notify_all();
}

void Latch::Wait()
{
    Lock lock(mMutex);
    mCV.wait(lock, [this] { return mSet.load(); });
}
