/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Debug Renderer
 */

#pragma once

#include "Multishader.hpp"
#include "Renderer.hpp"
#include "RendererResources.hpp"

namespace NFE {
namespace Renderer {

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
struct DebugRendererData
{
    // RendererTextureD3D11* texture;
    DebugPolyType polyType;
    UINT* indexQueue;
    UINT indexQueueSize;
    DebugVertex* vertexQueue;
    UINT vertexQueueSize;
};

// TODO: remove inheritance
class DebugRenderer
{
public:
    DebugRenderer();
    ~DebugRenderer();

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
    void DrawQuad(NFE_CONTEXT_ARG, const Vector& pos, ITexture* pTexture, const UINT color);
};

} // namespace Renderer
} // namespace NFE
