/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Common utilities for D3D11 renderer.
 */

#pragma once

#include "nfCommon/Language.hpp"


#ifndef D3D_SAFE_RELEASE
#define D3D_SAFE_RELEASE(x) { if (x) {(x)->Release(); (x)=0;} }
#endif // D3D_SAFE_RELEASE

#ifdef _DEBUG
// D3D debugging will be enabled only in "Debug" project's configuration
#define D3D_DEBUGGING
#endif // _DEBUG


namespace NFE {
namespace Renderer {

/**
 * DirectX API error handling
 * @param hr           Result of a D3D call
 * @param srcFile,line Source file name and line with the call
 */
HRESULT D3DError(HRESULT hr, const char* srcFile, int line);

#ifndef D3D_CALL_CHECK
#define D3D_CALL_CHECK(x) D3DError((x), __FILE__, __LINE__)
#endif

// helper class template for automatic releasing of Direct3D objects
template<typename T>
class D3DPtr final
{
    NFE_MAKE_NONCOPYABLE(D3DPtr)

public:
    T* pointer;

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

    operator bool() const
    {
        return pointer != nullptr;
    }
};

} // namespace Renderer
} // namespace NFE
