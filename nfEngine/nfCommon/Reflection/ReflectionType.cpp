/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's Type class.
 */

#include "../PCH.hpp"
#include "Reflection.h"


namespace NFE {
namespace RTTI {

bool Type::DefineInheritance(Type* parent, Type* child)
{
    if (child->mParent)
    {
        // TODO multiple inheritance is not supported yet
        return false;
    }

    parent->mChildTypes.push_back(child);
    child->mParent = parent;
    return true;
}

} // namespace RTTI
} // namespace NFE
