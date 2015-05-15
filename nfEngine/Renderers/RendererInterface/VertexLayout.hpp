/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

// TODO: remove
class IShader;

/**
 * Vertex layout element description.
 */
struct VertexLayoutElement
{
    ElementFormat format;
    int size;

    // TODO: instancing support
};

/**
 * Vertex layout description.
 */
struct VertexLayoutDesc
{
    VertexLayoutElement* elements;
    int numElements;

    /*
      This parameter is present only due to weird D3D11 API.
      It must be removed in final version. A nice workaround is to generate dummy vertex shader
      only for ID3D11Device::CreateInputLayout call.
    */
    IShader* vertexShader;
};

class IVertexLayout
{
public:
    virtual ~IVertexLayout() {}
};

} // namespace Renderer
} // namespace NFE
