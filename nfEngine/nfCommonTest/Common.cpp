/**
* @brief Definitions of common functions and classes.
*/

#include "stdafx.hpp"
#include "Common.hpp"

Latch::Latch()
{
    set = false;
}

void Latch::Set()
{
    std::unique_lock<std::mutex> lock(mutex);
    set = true;
    cv.notify_all();
}

void Latch::Wait()
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return set.load(); });
}
