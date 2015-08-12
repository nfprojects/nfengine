/**
* @file
* @author  LKostyra (costyrra.xl@gmail.com)
* @brief   VertexLayout OpenGL 4 declaration
*/

#pragma once

#include "../RendererInterface/VertexLayout.hpp"

namespace NFE {
namespace Renderer {

class VertexLayout: public IVertexLayout
{
    friend class CommandBuffer;

    VertexLayoutDesc mDesc;
public:
    VertexLayout();
    ~VertexLayout();
    bool Init(const VertexLayoutDesc& desc);
};

} // namespace Renderer
} // namespace NFE
