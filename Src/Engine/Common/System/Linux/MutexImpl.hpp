/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Reader/writer lock (mutex) Linux implementation.
 */

#pragma once

#include "../Mutex.hpp"
#include "../Assertion.hpp"

#include <pthread.h>

namespace NFE {
namespace Common {


Mutex::Mutex()
{
    int result = ::pthread_mutex_init(&mMutexObject, nullptr);
    NFE_ASSERT(result == 0, "Mutex initialization failed");
}

Mutex::~Mutex()
{
    int result = ::pthread_mutex_destroy(&mMutexObject);
    NFE_ASSERT(result == 0, "Mutex destruction failed");
}

bool Mutex::TryAcquireExclusive()
{
    return 0 == ::pthread_mutex_trylock(&mMutexObject);
}

void Mutex::AcquireExclusive()
{
    int result = ::pthread_mutex_lock(&mMutexObject);
    NFE_ASSERT(result == 0, "Acquiring exclusive lock failed");
}

void Mutex::ReleaseExclusive()
{
    int result = ::pthread_mutex_unlock(&mMutexObject);
    NFE_ASSERT(result == 0, "Releasing exclusive lock failed");
}


} // namespace Common
} // namespace NFE
