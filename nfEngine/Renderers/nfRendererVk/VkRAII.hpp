#pragma once

#include "Device.hpp"


namespace NFE {
namespace Renderer {

template <typename T>
class VkRAII
{
    template <typename Type>
    using DeleterFPtr = void (*)(VkDevice, Type, const VkAllocationCallbacks*);

    T mObject;
    DeleterFPtr<T> mDeleter;

public:
    // Default constructor, carrying no object at all
    VkRAII()
        : mObject(VK_NULL_HANDLE)
        , mDeleter(nullptr)
    {
    }

    // Constructor capturing object and deleter pointer
    VkRAII(T object, DeleterFPtr<T> deleter)
        : mObject(object)
        , mDeleter(deleter)
    {
    }

    // Destructor, will clean up the object after call
    ~VkRAII()
    {
        if (mObject != VK_NULL_HANDLE && mDeleter)
            mDeleter(gDevice->GetDevice(), mObject, nullptr);
    }

    // We don't alow copying of objects for now.
    // Implementing this would require making a SharedPtr-like mechanism.
    VkRAII(const VkRAII& other) = delete;
    VkRAII& operator=(const VkRAII& other) = delete;

    // Move constructor and operator, helpful in some cases
    VkRAII(VkRAII&& other)
        : mObject(other.mObject)
        , mDeleter(other.mDeleter)
    {
        other.mObject = VK_NULL_HANDLE;
        other.mDeleter = nullptr;
    }

    VkRAII& operator=(VkRAII&& other)
    {
        mObject = other.mObject;
        mDeleter = other.mDeleter;

        other.mObject = VK_NULL_HANDLE;
        other.mDeleter = nullptr;

        return *this;
    }

    // To use VkRAII like a regular Vulkan handle
    operator T()
    {
        return mObject;
    }

    // To check whether the object carries a valid Vulkan object, useful for error checks
    operator bool()
    {
        return (mObject != VK_NULL_HANDLE) && (mDeleter != nullptr);
    }
};

} // namespace Renderer
} // namespace NFE
