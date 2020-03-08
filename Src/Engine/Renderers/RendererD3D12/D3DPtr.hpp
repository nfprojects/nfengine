/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D smart pointer definition
 */

#pragma once


#include "Engine/Common/Containers/UniquePtr.hpp"


#ifndef D3D_SAFE_RELEASE
#define D3D_SAFE_RELEASE(x) { if (x) {(x)->Release(); (x)=0;} }
#endif // D3D_SAFE_RELEASE


namespace NFE {
namespace Renderer {

template<typename T>
struct D3DPointerDeleter
{
    static_assert(std::is_base_of<IUnknown, T>::value, "D3DPointerDeleter only accepts IUnknown-based types");

    static void Delete(T* pointer)
    {
        if (pointer)
        {
            pointer->Release();
        }
    }
};


// helper class template for automatic releasing of Direct3D objects
template<typename T>
using D3DPtr = Common::UniquePtr<T, D3DPointerDeleter<T>>;


} // namespace Renderer
} // namespace NFE
