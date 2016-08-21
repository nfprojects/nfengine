/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D smart pointer definition
 */

#pragma once


#ifndef D3D_SAFE_RELEASE
#define D3D_SAFE_RELEASE(x) { if (x) {(x)->Release(); (x)=0;} }
#endif // D3D_SAFE_RELEASE

namespace NFE {
namespace Renderer {

// helper class template for automatic releasing of Direct3D objects
template<typename T>
class D3DPtr
{
    static_assert(std::is_base_of<IUnknown, T>::value, "D3DPtr only accepts IUnknown-based types");

    T* pointer;

    D3DPtr(const D3DPtr&) = delete;
    D3DPtr& operator=(const D3DPtr&) = delete;

public:
    D3DPtr() : pointer(nullptr)
    {
    }

    D3DPtr(T* ptr)
    {
        pointer = ptr;
    }

    D3DPtr(D3DPtr<T>&& rhs)
    {
        pointer = rhs.pointer;
        rhs.pointer = nullptr;
    }

    ~D3DPtr()
    {
        reset();
    }

    D3DPtr& operator=(T* ptr)
    {
        if (pointer != ptr)
        {
            if (nullptr != pointer)
                pointer->Release();
            pointer = ptr;
        }
        return *this;
    }

    D3DPtr& operator=(D3DPtr&& ptr)
    {
        reset();
        pointer = ptr.pointer;
        ptr.pointer = nullptr;
        return *this;
    }

    T** operator&()
    {
        return &pointer;
    }

    T* operator->() const
    {
        return pointer;
    }

    T& operator*() const
    {
        return *pointer;
    }

    T* get() const
    {
        return pointer;
    }

    void reset(T* newPtr = nullptr)
    {
        if (pointer)
        {
            pointer->Release();
            pointer = nullptr;
        }
        pointer = newPtr;
    }

    operator bool() const
    {
        return pointer != nullptr;
    }
};

} // namespace Renderer
} // namespace NFE
