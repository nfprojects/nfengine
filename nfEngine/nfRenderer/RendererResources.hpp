/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of high-level renderer resources.
 */

#pragma once

#include "nfRenderer.hpp"
#include "../Renderers/RendererInterface/Device.hpp"

#include "nfCommon/Math/Matrix4.hpp"


namespace NFE {
namespace Renderer {

struct MeshVertex
{
    Math::Float3 position;  // TODO 16 bit coordinates
    Math::Float2 texCoord;  // TODO 16 bit coordinates
    char normal[4];
    char tangent[4];
};

struct NFE_ALIGN(16) ShadowCameraRenderDesc
{
    Math::Matrix4 viewProjMatrix;
    Math::Vector lightPos;
};

struct NFE_ALIGN(16) CameraRenderDesc
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
};

struct RendererMaterial
{
    RendererMaterialLayer layers[1];
    // TODO multiple texture layers support
};


} // namespace Renderer
} // namespace NFE
