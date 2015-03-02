#pragma once
#include "nfRenderer.hpp"
#include "Multishader.hpp"
#include "../nfCore/RenderCommand.hpp"
#include "../nfCore/Renderer.hpp"

namespace NFE {
namespace Render {

#define MAX_BUFFERED_INSTANCES (8192)

class RenderContextD3D11 : public IRenderContext
{
    //currently bound shaders
    ID3D11VertexShader* mBoundVS;
    ID3D11PixelShader* mBoundPS;
    ID3D11GeometryShader* mBoundGS;
    ID3D11DomainShader* mBoundDS;
    ID3D11HullShader* mBoundHS;
    ID3D11ComputeShader* mBoundCS;

    // owning renderer
    RendererD3D11* mRenderer;

public:
    ID3D11DeviceContext* D3DContext;
    ID3D11CommandList* commandList;

    RenderContextD3D11();
    ~RenderContextD3D11();
    void Init(uint32 id, RendererD3D11* pRenderer, ID3D11DeviceContext* pContext = nullptr);
    void Release();

    int BindShader(Multishader* pShader, const UINT* pMacroValues);
    int ResetShader(ShaderType type);
    RendererD3D11* GetRenderer() const;

    void Begin();
    void End();

    /*
        Execute deferred context pCtx.
        Setting 'saveState' to true restores previous context state after execution
    */
    bool Execute(IRenderContext* pContext, bool saveState = false);
};

} // namespace Render
} // namespace NFE
