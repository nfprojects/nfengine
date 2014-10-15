#pragma once

namespace NFE {
namespace Render {

//#pragma warning(disable: 4324)
//#pragma warning(disable: 4099)
#pragma warning(disable: 4251)

// DLL import / export macro
#ifdef NFRENDERERD3D11_EXPORTS
#define NFRENDERER_API __declspec(dllexport)
#else
#define NFRENDERER_API __declspec(dllimport)
#endif

// define basic data types
typedef unsigned __int64 uint64;
typedef __int64 int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uchar;


#ifdef NFRENDERERD3D11_EXPORTS
//safe direct3d interface object release
#ifndef D3D_SAFE_RELEASE
#define D3D_SAFE_RELEASE(x) { if (x) {(x)->Release(); (x)=0;} }
#endif


// DirectX API error handling
HRESULT D3DError(HRESULT hr, const char* pStr, const char* pFile, int line, bool critical);

#ifndef D3D_CHECK
// Check HRESULT, show error message when failed. Use for critical D3D calls
#define D3D_CHECK(x) D3DError((x), #x, __FILE__, __LINE__, true);
#endif

#ifndef D3D_LOG
// Check HRESULT, log error message when failed. Use for non-critical D3D calls
#define D3D_LOG(x) D3DError((x), #x, __FILE__, __LINE__, false);
#endif

class RenderContextD3D11;
class PostProcessRendererD3D11;
class GuiRendererD3D11;
class LightsRendererD3D11;
class DebugRendererD3D11;
class GBufferRendererD3D11;
class ShadowRendererD3D11;
#endif

class IRenderer;
extern "C" NFRENDERER_API IRenderer* GetInstance();

} // namespace Render
} // namespace NFE
