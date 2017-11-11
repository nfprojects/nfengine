/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of high-level renderer resources.
 */

#pragma once

#include "nfRenderer.hpp"
#include "../Renderers/RendererInterface/Device.hpp"

#include "nfCommon/Math/Matrix4.hpp"
#include "nfCommon/Math/Box.hpp"


namespace NFE {
namespace Renderer {

struct MeshVertex
{
    Math::Float3 position;  // TODO 16 bit
    Math::Float2 texCoord;  // TODO 16 bit
    int8 normal[4];         // normalized to -1...1 range
    int8 tangent[4];        // normalized to -1...1 range
};

struct NFE_ALIGN(16) ShadowCameraDesc
{
    Math::Matrix4 viewProjMatrix;
    Math::Vector4 lightPos;
};

struct NFE_ALIGN(16) CameraDesc
{
    Math::Matrix4 matrix;        // global camera matrix (pos + orientation)
    Math::Matrix4 viewMatrix;
    Math::Matrix4 projMatrix;
    Math::Matrix4 secViewMatrix; // for motion blur

    Math::Vector4 velocity;
    Math::Vector4 angualrVelocity;

    Math::Vector4 screenScale;
    float fov;
};

/**
 *  Structure describing material layer information.
 *  Used by GBuffer renderer.
 */
struct RendererMaterialLayer
{
    ResourceBindingInstancePtr bindingInstance;

    Math::Float4 diffuseColor;
    Math::Float4 specularColor; // x - factor, w - power
    Math::Float4 emissionColor;

    Math::Float2 scaling; // UV scaling factor for this layer
};

/**
 * Material - list of layers
 */
struct RendererMaterial
{
    RendererMaterialLayer layers[1];
    // TODO multiple layers support
    // TODO layers blending
};

/**
 * Information about a part of a mesh that's using the same material for each triangle
 */
struct RenderSubMesh
{
    Math::Box localBox;
    Common::SharedPtr<RendererMaterial> material; // TODO weak ptr?
    uint32 indexOffset;
    uint32 trianglesCount;
};

/**
 * Mesh - composition of submeshes
 */
struct RenderMesh
{
    // TODO sharing buffers with other render meshes
    Renderer::BufferPtr vertexBuffer;
    Renderer::BufferPtr indexBuffer;

    Common::DynArray<RenderSubMesh> subMeshes;
};


} // namespace Renderer
} // namespace NFE
