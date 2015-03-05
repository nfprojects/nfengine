/**
* @file    Common.hpp
* @author  Witek902 (witek902@gmail.com)
* @brief   Common utilities for D3D11 renderer.
*/

#pragma once

#define D3D_CHECK(x) x

#ifndef D3D_SAFE_RELEASE
#define D3D_SAFE_RELEASE(x) { if (x) {(x)->Release(); (x)=0;} }
#endif

namespace NFE {
namespace Renderer {

// helper class template for automatic releasing of Direct3D objects
template<typename T>
class D3DPtr
{
private:
    T* pointer;

    /// disable "copy" methods
    D3DPtr(const D3DPtr&);
    D3DPtr& operator=(const D3DPtr&);

public:
    D3DPtr() : pointer(nullptr) {}

    D3DPtr(T* ptr)
    {
        static_assert(std::is_base_of<IUnknown, T>::value, "D3DPtr only accepts IUnknown-based types");
        pointer = ptr;
    }

    D3DPtr(D3DPtr<T>&& rhs)
    {
        reset();
        pointer = rhs.pointer;
    }

    ~D3DPtr()
    {
        reset();
    }

    D3DPtr& operator=(T* ptr)
    {
        if (pointer != ptr)
        {
            if (nullptr != ptr)
                ptr->Release();
            pointer = ptr;
        }
        return *this;
    }

    D3DPtr& operator=(D3DPtr&& ptr)
    {
        std::swap(pointer, ptr.pointer);
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

    void reset()
    {
        if (pointer)
        {
            pointer->Release();
            pointer = nullptr;
        }
    }
};

} // namespace Renderer
} // namespace NFE
