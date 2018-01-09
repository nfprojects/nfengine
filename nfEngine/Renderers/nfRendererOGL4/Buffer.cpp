/**
* @file
* @author  LKostyra (costyrra.xl@gmail.com)
* @brief   Declaration of OpenGL 4 renderer's buffer.
*/

#include "PCH.hpp"

#include "Buffer.hpp"

namespace NFE {
namespace Renderer {

Buffer::Buffer()
    : mSize(0)
    , mType(BufferType::Vertex)
    , mAccess(BufferAccess::GPU_ReadOnly)
    , mBuffer(GL_NONE)
{
}

Buffer::~Buffer()
{
    if (mBuffer)
    {
        glDeleteBuffers(1, &mBuffer);
        mBuffer = 0;
    }

    mSize = 0;
}

bool Buffer::Init(const BufferDesc& desc)
{
    GLenum bufferTarget;
    GLenum bufferAccess;
    mSize = desc.size;
    mAccess = desc.access;
    mType = desc.type;

    // translate type and access mode
    switch (mType)
    {
    case BufferType::Vertex:
        bufferTarget = GL_ARRAY_BUFFER;
        break;
    case BufferType::Index:
        bufferTarget = GL_ELEMENT_ARRAY_BUFFER;
        break;
    case BufferType::Constant:
        bufferTarget = GL_UNIFORM_BUFFER;
        break;
    default:
        NFE_LOG_ERROR("Invalid buffer type.");
        return false;
    }

    // NOTE According to manual, even though these parameters can be provided, they don't do much.
    //      This is just a hint for OGL on how the buffer will be accessed.
    switch (mAccess)
    {
    case BufferAccess::GPU_ReadOnly:
    case BufferAccess::CPU_Read:
        bufferAccess = GL_STATIC_DRAW;
        break;
    case BufferAccess::GPU_ReadWrite:
    case BufferAccess::CPU_Write:
        bufferAccess = GL_DYNAMIC_DRAW;
        break;
    default:
        NFE_LOG_ERROR("Invalid buffer access mode.");
        return false;
    }

    glGenBuffers(1, &mBuffer);
    if (!mBuffer)
    {
        NFE_LOG_ERROR("Failed to create Buffer Object.");
        return false;
    }

    // bind buffer and upload data to it
    glBindBuffer(bufferTarget, mBuffer);
    glBufferData(bufferTarget, mSize, desc.initialData, bufferAccess);
    glBindBuffer(bufferTarget, GL_NONE);

    return true;
}

} // namespace Renderer
} // namespace NFE
