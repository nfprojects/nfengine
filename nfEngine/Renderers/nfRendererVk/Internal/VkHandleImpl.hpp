#pragma once

#include "API/Device.hpp"


namespace NFE {
namespace Renderer {

template <typename T>
VkHandle<T>::VkHandle()
    : mObject(VK_NULL_HANDLE)
    , mDeleter(nullptr)
{
}

template <typename T>
VkHandle<T>::VkHandle(T object, VkHandle<T>::DeleterFPtr<T> deleter)
    : mObject(object)
    , mDeleter(deleter)
{
}

template <typename T>
VkHandle<T>::~VkHandle()
{
    if (mObject != VK_NULL_HANDLE && mDeleter)
        mDeleter(gDevice->GetDevice(), mObject, nullptr);
}

// Move constructor and operator, helpful in some cases
template <typename T>
VkHandle<T>::VkHandle(VkHandle<T>&& other)
    : mObject(other.mObject)
    , mDeleter(other.mDeleter)
{
    other.mObject = VK_NULL_HANDLE;
    other.mDeleter = nullptr;
}

template <typename T>
VkHandle<T>& VkHandle<T>::operator=(VkHandle&& other)
{
    mObject = other.mObject;
    mDeleter = other.mDeleter;

    other.mObject = VK_NULL_HANDLE;
    other.mDeleter = nullptr;

    return *this;
}

template <typename T>
VkHandle<T>::operator T()
{
    return mObject;
}

template <typename T>
VkHandle<T>::operator bool()
{
    return (mObject != VK_NULL_HANDLE) && (mDeleter != nullptr);
}

} // namespace Renderer
} // namespace NFE
