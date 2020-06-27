#pragma once

#include "../nfCommon.hpp"
#include "../Containers/StaticArray.hpp"
#include "../Containers/StringView.hpp"

namespace NFE {
namespace RTTI {

/**
 * Represents a path to a member in a serializable object.
 */
struct MemberPath
{
    static constexpr uint32 MaxDepth = 16; // should be enough for now

    enum class ElementType : uint8
    {
        Name,
        Index,
    };

    struct Element
    {
        ElementType type;
        Common::StringView name;
        uint32 index;

        NFE_FORCE_INLINE Element(const Element&) = default;
        NFE_FORCE_INLINE Element(const Common::StringView name) : type(ElementType::Name), name(name) { }
        NFE_FORCE_INLINE Element(const uint32 index) : type(ElementType::Index), index(index) { }

        NFE_FORCE_INLINE bool operator == (const Element& other) const
        {
            if (type == other.type)
            {
                return type == ElementType::Name ? name == other.name : index == other.index;
            }
            return false;
        }

        NFE_FORCE_INLINE bool operator != (const Element& other) const
        {
            if (type == other.type)
            {
                return type == ElementType::Name ? name != other.name : index != other.index;
            }
            return true;
        }
    };

    Common::StaticArray<Element, MaxDepth> elements;

    NFE_FORCE_INLINE MemberPath() = default;
    NFE_FORCE_INLINE MemberPath(const MemberPath&) = default;
    NFE_FORCE_INLINE MemberPath(MemberPath&&) = default;

    NFE_FORCE_INLINE MemberPath(const char* name)
    {
        elements.PushBack(Element(name));
    }

    NFE_FORCE_INLINE MemberPath(const Common::StringView name)
    {
        elements.PushBack(Element(name));
    }

    NFE_FORCE_INLINE MemberPath& operator[] (const char* name)
    {
        elements.PushBack(Element(name));
        return *this;
    }

    NFE_FORCE_INLINE MemberPath& operator[] (const uint32 index)
    {
        elements.PushBack(Element(index));
        return *this;
    }

    NFE_FORCE_INLINE bool operator == (const MemberPath& other) const
    {
        return elements.GetView() == other.elements.GetView();
    }

    NFE_FORCE_INLINE bool operator != (const MemberPath& other) const
    {
        return elements.GetView() != other.elements.GetView();
    }
};

} // namespace RTTI
} // namespace NFE
