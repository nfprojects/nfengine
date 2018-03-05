#pragma once

namespace NFE {
namespace Renderer {

template <typename T>
class VkRAII
{
    template <typename Type>
    using DeleterFPtr = void (*)(VkDevice, Type, const VkAllocationCallbacks*);

    VkDevice mDevice;
    T mObject;
    DeleterFPtr<T> mDeleter;

public:
    // Default constructor, carrying no object at all
    VkRAII()
        : mDevice(VK_NULL_HANDLE)
        , mObject(VK_NULL_HANDLE)
        , mDeleter(nullptr)
    {
    }

    // Constructor capturing object, its device and deleter pointer
    VkRAII(VkDevice device, T object, DeleterFPtr<T> deleter)
        : mDevice(device)
        , mObject(object)
        , mDeleter(deleter)
    {
    }

    // Destructor, will clean up the object after call
    ~VkRAII()
    {
        mDeleter(mDevice, mObject, nullptr);
    }

    // We don't alow copying of objects for now.
    // Implementing this would require making a SharedPtr-like mechanism.
    VkRAII(const VkRAII& other) = delete;
    VkRAII& operator=(const VkRAII& other) = delete;

    // Move constructor and operator, helpful in some cases
    VkRAII(VkRAII&& other)
        : mDevice(other.mDevice)
        , mObject(other.mObject)
        , mDeleter(other.mDeleter)
    {
        other.mDevice = VK_NULL_HANDLE;
        other.mObject = VK_NULL_HANDLE;
        other.mDeleter = nullptr;
    }

    VkRAII& operator=(VkRAII&& other)
    {
        mDevice = other.mDevice;
        mObject = other.mObject;
        mDeleter = other.mDeleter;

        other.mDevice = VK_NULL_HANDLE;
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
