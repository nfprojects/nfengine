#pragma once

#include "RenderSceneObject.hpp"
#include "../../Common/Containers/SharedPtr.hpp"


namespace NFE {
namespace Renderer {

class Mesh;
using MeshPtr = Common::SharedPtr<Mesh>;

class SceneObject_Mesh : public SceneObject
{
public:

private:
    MeshPtr mMesh;
};


} // namespace Renderer
} // namespace NFE
