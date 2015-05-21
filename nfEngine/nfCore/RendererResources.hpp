#pragma once
#include "Renderer.hpp"

namespace NFE {
namespace Renderer {

    /*
// geometry buffer for deferred shading
class GeometryBuffer
{
public:
    static const int mLayersNum = 4;
    uint32 width, height;

    GeometryBuffer();
    ~GeometryBuffer();
    void Release();
    int Resize(uint32 width_, uint32 height_);
};

// shadow map
class ShadowMapD3D11 : public IShadowMap
{
public:
    uint32 size;
    uint32 splits;
    Type type;

    ShadowMapD3D11();
    ~ShadowMapD3D11();
    void Release();
    int Resize(uint32 size_, Type type_, uint32 splits_ = 1);
    uint32 GetSize() const;
};

*/

} // namespace Renderer
} // namespace NFE
