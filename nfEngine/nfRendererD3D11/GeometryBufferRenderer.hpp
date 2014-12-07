#pragma once
#include "nfRendererD3D11.hpp"
#include "Multishader.hpp"
#include "../nfCore/RenderCommand.hpp"
#include "../nfCore/Renderer.hpp"

namespace NFE {
namespace Render {

struct GBufferRendererD3D11Data
{
    const RendererMaterial* mCurrMaterial;
    InstanceData* mInstanceData;
};


class GBufferRendererD3D11 : public IGBufferRenderer
{
private:
    ID3D11DepthStencilState* mDepthStencilState;
    ID3D11RasterizerState* mRasterizerState;
    ID3D11SamplerState* mSampler;
    ID3D11InputLayout* mInputLayout;

    ID3D11Buffer* mInstancesVB;
    ID3D11Buffer* mMaterialCBuffer;
    ID3D11Buffer* mGlobalCBuffer;
    ID3D11Buffer* mPerInstanceCBuffer;

    Multishader mShaderVS;
    Multishader mShaderPS;
    Multishader mShaderGS;


    // TODO: the following must be per-context !!!
    InstanceData* mInstanceData;

public:
    GBufferRendererD3D11();
    ~GBufferRendererD3D11();

    Result Init(RendererD3D11* pRenderer);
    void Release();
    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget);
    void SetCamera(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera);
    void SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial);
    void Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer);
};

} // namespace Render
} // namespace NFE
