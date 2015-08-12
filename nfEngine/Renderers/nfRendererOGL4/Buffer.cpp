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
        LOG_ERROR("Uniforms (aka Constant Buffers) are not yet implemented!");
        return false;
    default:
        LOG_ERROR("Invalid buffer type.");
        return false;
    }

    // NOTE According to manual, even though these parameters can be provided, they don't do much.
    //      This is just a hint for OGL on how the buffer will be accessed.
    switch (mAccess)
    {
    case BufferAccess::GPU_ReadOnly:
        bufferAccess = GL_STATIC_DRAW;
        break;
    case BufferAccess::GPU_ReadWrite:
        bufferAccess = GL_DYNAMIC_DRAW;
        break;
    case BufferAccess::CPU_Read:
        LOG_ERROR("CPU Buffer Access is not yet implemented!");
        return false;
    case BufferAccess::CPU_Write:
        LOG_ERROR("CPU Buffer Access is not yet implemented!");
        return false;
    default:
        LOG_ERROR("Invalid buffer access mode.");
        return false;
    }

    glGenBuffers(1, &mBuffer);
    if (!mBuffer)
    {
        LOG_ERROR("Failed to create Buffer Object.");
        return false;
    }

    // bind buffer and upload data to it
    glBindBuffer(bufferTarget, mBuffer);
    glBufferData(bufferTarget, mSize, desc.initialData, bufferAccess);

    return true;
}

} // namespace Renderer
} // namespace NFE
