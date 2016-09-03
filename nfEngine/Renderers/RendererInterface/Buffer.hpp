/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

/**
 * Description of renderer's buffer
 */
struct BufferDesc
{
    BufferType type;
    BufferMode mode;
    const void* initialData;
    size_t size;
    const char* debugName;   //< optional debug name

    BufferDesc()
        : type(BufferType::Vertex)
        , mode(BufferMode::Static)
        , initialData(nullptr)
        , size(0)
        , debugName(nullptr)
    {}
};

/**
 * GPU data buffer which can act as a vertex, index or a constant buffer.
 */
class IBuffer
{
public:
    virtual ~IBuffer() {}
};

} // namespace Renderer
} // namespace NFE
