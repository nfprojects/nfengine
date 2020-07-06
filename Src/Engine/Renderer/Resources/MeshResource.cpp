#include "PCH.hpp"
#include "MeshResource.hpp"
#include "../../Common/Reflection/ReflectionClassDefine.hpp"
#include "../../Common/Reflection/Types/ReflectionDynArrayType.hpp"


NFE_DEFINE_CLASS(NFE::Renderer::MeshPart)
{
    NFE_CLASS_MEMBER(firstVertex);
    NFE_CLASS_MEMBER(firstIndex);
    NFE_CLASS_MEMBER(numTriangles);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(NFE::Renderer::MeshLodLevel)
{
    NFE_CLASS_MEMBER(maxDistance);
    NFE_CLASS_MEMBER(parts);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Renderer::Mesh)
{
    NFE_CLASS_PARENT(NFE::IObject);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Renderer {


} // namespace Scene
} // namespace NFE
