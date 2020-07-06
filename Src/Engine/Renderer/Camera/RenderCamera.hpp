#pragma once

#include "../RendererApi.hpp"
#include "../../Common/Math/Matrix4.hpp"


namespace NFE {
namespace Renderer {

class NFE_RENDERER_API Camera
{
public:

private:
    Math::Matrix4 mViewMatrix;
    Math::Matrix4 mProjectionMatrix;
};


} // namespace Renderer
} // namespace NFE
