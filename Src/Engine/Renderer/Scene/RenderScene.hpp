#pragma once

#include "RenderSceneObject.hpp"
#include "../../Common/Math/Vec4f.hpp"


namespace NFE {
namespace Renderer {

class RenderGraphContext;

struct SceneEnvironment
{
    Math::Vec4f backgroundColor;
};

class Scene
{
public:
    void RenderGBuffer(RenderGraphContext& context);
};


} // namespace Renderer
} // namespace NFE
