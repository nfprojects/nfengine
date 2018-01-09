/**
* @file
* @author  LKostyra (costyrra.xl@gmail.com)
* @brief   VertexLayout OpenGL 4 definition
*/

#include "PCH.hpp"

#include "Defines.hpp"
#include "VertexLayout.hpp"

namespace NFE {
namespace Renderer {

VertexLayout::VertexLayout()
{
}

VertexLayout::~VertexLayout()
{
    delete[] mDesc.elements;
}

bool VertexLayout::Init(const VertexLayoutDesc& desc)
{
    // OGL needs only elements descriptors and their count
    mDesc.numElements = desc.numElements;
    mDesc.elements = new (std::nothrow) VertexLayoutElement[mDesc.numElements];
    if (!mDesc.elements)
    {
        NFE_LOG_ERROR("Unable to copy Vertex Layout Elements");
        return false;
    }

    for (int i = 0; i < mDesc.numElements; ++i)
        mDesc.elements[i] = desc.elements[i];

    // verify if the copy is correct
    if (memcmp(mDesc.elements, desc.elements, sizeof(VertexLayoutElement)*mDesc.numElements))
    {
        NFE_LOG_ERROR("Copying Vertex Layout elements failed.");
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
