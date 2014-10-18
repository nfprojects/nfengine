#pragma once
#include "nfRendererD3D11.hpp"
#include "Multishader.hpp"
#include "../nfCore/Renderer.hpp"
#include "RendererResources.hpp"

namespace NFE {
namespace Render {

using namespace Math;

//vertex buffer capacity
#define DEBUG_VB_SIZE (1024)
#define DEBUG_IB_SIZE (2048)

enum class DebugPolyType
{
    Point = 0,
    Line,
    Triangle,
    TriangleTex,
    Mesh,
};

struct DebugStateDesc
{
    Matrix viewMatrix;
    Matrix projMatrix;
};

struct DebugCBuffer
{
    Matrix viewMatrix;
    Matrix projMatrix;
};

struct DebugVertex
{
    Float3 pos;
    UINT color;
    Float2 texCoord;
};

// debug renderer per context data
struct DebugRendererD3D11Data
{
    RendererTextureD3D11* texture;
    DebugPolyType polyType;
    UINT* indexQueue;
    UINT indexQueueSize;
    DebugVertex* vertexQueue;
    UINT vertexQueueSize;
};


class DebugRendererD3D11 : public IDebugRenderer
{
    ID3D11DepthStencilState* mDepthStencilState;
    ID3D11RasterizerState* mRasterizerState;
    ID3D11SamplerState* mSampler;
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
    ID3D11InputLayout* mInputLayout;
    ID3D11InputLayout* mMeshInputLayout;
    ID3D11Buffer* mCBuffer;

    Multishader mShaderVS;
    Multishader mShaderPS;
    Multishader mMeshShaderVS;
    Multishader mMeshShaderPS;

    RendererTextureD3D11* mTexture;
    DebugPolyType mPolyType;
    UINT* mIndexQueue;
    UINT mIndexQueueSize;
    DebugVertex* mVertexQueue;
    UINT mVertexQueueSize;

    void Flush(NFE_CONTEXT_ARG);

public:
    DebugRendererD3D11();
    ~DebugRendererD3D11();

    Result Init(RendererD3D11* pRenderer);
    void Release();

    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget);
    void SetCamera(NFE_CONTEXT_ARG, const Matrix& viewMatrix,
                   const Matrix& projMatrix); // TODO: CameraRenderDesc
    void DrawLine(NFE_CONTEXT_ARG, const Vector& A, const Vector& B, const UINT color);
    void DrawLine(NFE_CONTEXT_ARG, const Float3& A, const Float3& B, const UINT color);
    void DrawBox(NFE_CONTEXT_ARG, const Box& box, const UINT color);
    void DrawFilledBox(NFE_CONTEXT_ARG, const Box& box, const UINT color);
    void DrawFrustum(NFE_CONTEXT_ARG, const Frustum& frustum, const UINT color);
    void DrawQuad(NFE_CONTEXT_ARG, const Vector& pos, IRendererTexture* pTexture, const UINT color);
    //void SetMaterial(NFE_CONTEXT_ARG, const Material* pMaterial);
    //void DrawMesh(NFE_CONTEXT_ARG, const Mesh* pMesh);
};

} // namespace Render
} // namespace NFE
