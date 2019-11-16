#include "PCH.h"
#include "MaterialParameter.h"


NFE_BEGIN_DEFINE_CLASS(NFE::RT::MaterialParameter)
{
    NFE_CLASS_MEMBER(baseValue).Min(0.0f).Max(1.0f);
    // TODO texture
}
NFE_END_DEFINE_CLASS()


NFE_BEGIN_DEFINE_CLASS(NFE::RT::ColorMaterialParameter)
{
    NFE_CLASS_MEMBER(baseValue);
    // TODO texture
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {


} // namespace RT
} // namespace NFE
