/**
* @file    Common.hpp
* @author  Witek902 (witek902@gmail.com)
* @brief   Common utilities for D3D11 renderer.
*/

#pragma once

#ifndef D3D_SAFE_RELEASE
#define D3D_SAFE_RELEASE(x) { if (x) {(x)->Release(); (x)=0;} }
#endif

namespace NFE {
namespace Renderer {

/**
 * DirectX API error handling
 */
HRESULT D3DError(HRESULT hr, const char* pStr, const char* pFile, int line, bool critical);

#ifndef D3D_CHECK
    // Check HRESULT, show error message when failed. Use for critical D3D calls
#define D3D_CHECK(x) D3DError((x), #x, __FILE__, __LINE__, true);
#endif

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
