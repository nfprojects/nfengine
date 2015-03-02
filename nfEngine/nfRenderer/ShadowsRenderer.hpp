#pragma once
#include "nfRenderer.hpp"
#include "Multishader.hpp"
#include "../nfCore/Renderer.hpp"
#include "../nfCore/RenderCommand.hpp"

namespace NFE {
namespace Render {

struct ShadowRendererD3D11Data
{

};


class ShadowRendererD3D11 : public IShadowRenderer
{
    ID3D11DepthStencilState* mDepthStencilState;
    ID3D11RasterizerState* mRasterizerState;
    ID3D11SamplerState* mSampler;
    ID3D11InputLayout* mInputLayout;

    ID3D11Buffer* mInstancesVB;
    ID3D11Buffer* mGlobalCBuffer;
    ID3D11Buffer* mPerInstanceCBuffer;

    Multishader mShaderVS;
    Multishader mShaderPS;

public:
    ShadowRendererD3D11();
    ~ShadowRendererD3D11();

    Result Init(RendererD3D11* pRenderer);
    void Release();
    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetDestination(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera, IShadowMap* pShadowMap,
                        uint32 faceID);
    void SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial);
    void Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer);
};

} // namespace Render
} // namespace NFE
