/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionArrayType.hpp"
#include "../ReflectionMemberPath.hpp"

namespace NFE {
namespace RTTI {

ArrayType::ArrayType(const Type* underlyingType)
    : mUnderlyingType(underlyingType)
{
    NFE_ASSERT(mUnderlyingType, "Array's underlying type cannot be null. Type: %s", GetName().Str());
}

bool ArrayType::GetMemberByPath(void* object, const MemberPath& path, const Type*& outMemberType, void*& outMemberData) const
{
    // empty path means we access 'this' object (not member)
    if (path.elements.Empty())
    {
        outMemberData = object;
        outMemberType = this;
        return true;
    }

    const MemberPath::Element pathElement = path.elements.Front();
    NFE_ASSERT(pathElement.type == MemberPath::ElementType::Index, "Array element must be referenced by index");

    const uint32 arraySize = GetArraySize(object);
    if (pathElement.index >= arraySize)
    {
        // array index out of bounds
        return false;
    }

    void* elementData = GetElementPointer(object, pathElement.index);
    NFE_ASSERT(elementData, "Invalid element data");

    // trim first element
    MemberPath childPath = path;
    childPath.elements.Erase(childPath.elements.Begin(), childPath.elements.Begin() + 1);

    // traverse child element
    return GetUnderlyingType()->GetMemberByPath(elementData, childPath, outMemberType, outMemberData);
}

} // namespace RTTI
} // namespace NFE
