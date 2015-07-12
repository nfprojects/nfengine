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
    ElementFormat format; //< element format
    int size;             //< element size
    int offset;           //< element size in bytes
    int vertexBufferId;   //< source vertex buffer slot
    bool perInstance;     //< is the vertex element constant for entire instance?
    int instanceDataStep; //< number of instances to be drawn with one perInstance data
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

    const char* debugName; //< optional debug name

    VertexLayoutDesc()
        : elements(nullptr)
        , numElements(0)
        , vertexShader(nullptr)
        , debugName(nullptr)
    {}
};

/**
 * Vertex layout.
 *
 * @details Represents data layout supplied to a vertex shader and vertex buffers bindings.
 */
class IVertexLayout
{
public:
    virtual ~IVertexLayout() {}
};

} // namespace Renderer
} // namespace NFE
