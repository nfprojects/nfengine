#include "PCH.h"
#include "ObjectEditor.h"
#include "Engine/Common/Reflection/ReflectionEnumMacros.hpp"
#include "Engine/Common/Reflection/Types/ReflectionUniquePtrType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionNativeArrayType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionDynArrayType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionClassType.hpp"
#include "Engine/Common/Math/LdrColor.hpp"
#include "Engine/Common/Math/HdrColor.hpp"
#include "Engine/Common/Math/Quaternion.hpp"

#include <imgui/imgui.h>

#pragma optimize("",off)

namespace NFE {

using namespace NFE::RTTI;
using namespace NFE::Common;

struct EditPropertyContext
{
    RTTI::MemberPath path;  // full path to edited property
    const char* name;       // name of property (does not apply to arrays)
    const Type* type;
    const void* data;
    const MemberMetadata* metadata = nullptr;
};

static bool EditObject_Internal(const EditPropertyContext& ctx, EditObjectContext& editCtx);

static bool EditObject_Internal_LdrColorRGB(const char* name, const Math::LdrColorRGB* color, EditObjectContext& editCtx)
{
    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, name);
    ImGui::NextColumn();

    // property value
    ImGui::SetNextItemWidth(-1);

    float values[3];
    values[0] = color->r / 255.0f;
    values[1] = color->g / 255.0f;
    values[2] = color->b / 255.0f;

    bool changed = false;
    if (ImGui::ColorEdit3("", values))
    {
        // TODO create variant
        // color->r = static_cast<uint8>(values[0] * 255.0f);
        // color->g = static_cast<uint8>(values[1] * 255.0f);
        // color->b = static_cast<uint8>(values[2] * 255.0f);
        NFE_FATAL("Not implemented");

        changed = true;
    }

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_Internal_HdrColorRGB(const char* name, const Math::HdrColorRGB* color, EditObjectContext& editCtx)
{
    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, name);
    ImGui::NextColumn();

    // property value
    ImGui::SetNextItemWidth(-1);

    Math::HdrColorRGB colorCopy = *color;

	bool changed = ImGui::ColorEdit3("", &(colorCopy.r), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

    if (changed)
    {
        colorCopy.r = Math::Max(0.0f, colorCopy.r);
        colorCopy.g = Math::Max(0.0f, colorCopy.g);
        colorCopy.b = Math::Max(0.0f, colorCopy.b);
        NFE_FATAL("Not implemented");

        // TODO create variant
    }

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_Internal_FloatVector(const char* name, const float* data, uint32 numComponents, EditObjectContext& editCtx)
{
    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, name);
    ImGui::NextColumn();

    // property value
    ImGui::SetNextItemWidth(-1);

    NFE_ASSERT(numComponents > 0 && numComponents <= 4, "");
    float dataCopy[4];
    memcpy(dataCopy, data, sizeof(float) * numComponents);

    bool changed = ImGui::InputScalarN("", ImGuiDataType_Float, dataCopy, numComponents, nullptr, nullptr, "%.4f");

    if (changed)
    {
        // TODO create variant
        NFE_FATAL("Not implemented");
    }

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_Internal_Quaternion(const char* name, const Math::Quaternion* quat, EditObjectContext& editCtx)
{
    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, name);
    ImGui::NextColumn();

    // property value
    ImGui::SetNextItemWidth(-1);

    bool changed = false;

    Math::Vec3f angles = quat->ToEulerAngles();
    angles *= 180.0f / NFE_MATH_PI;
    if (ImGui::InputFloat3("", &angles.x, 2))
    {
        angles *= NFE_MATH_PI / 180.0f;

        // TODO create variant
        //*quat = Math::Quaternion::FromEulerAngles(angles);
        NFE_FATAL("Not implemented");

        changed = true;
    }

    ImGui::NextColumn();

    return changed;
}

template<typename T, ImGuiDataType_ ImGuiDataType>
static bool EditObject_Internal_Fundamental_Typed(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    const T min = (ctx.metadata && ctx.metadata->HasMinRange()) ? static_cast<T>(ctx.metadata->min) : std::numeric_limits<T>::min();
    const T max = (ctx.metadata && ctx.metadata->HasMaxRange()) ? static_cast<T>(ctx.metadata->max) : std::numeric_limits<T>::max();

    const bool hasSlider = ctx.metadata && !ctx.metadata->HasNoSlider() && ctx.metadata->HasMinRange() && ctx.metadata->HasMaxRange();

    bool changed = false;

    T value = *reinterpret_cast<const T*>(ctx.data);

    if (!hasSlider)
    {
        changed = ImGui::InputScalar("##value", ImGuiDataType, &value);
    }
    else
    {
        const float power = std::is_floating_point_v<T> ? ctx.metadata->logScalePower : 1.0f;

        changed = ImGui::SliderScalar("##value", ImGuiDataType, &value, &min, &max, nullptr, power);
    }

    if (changed)
    {
        editCtx.objectAfter = Variant::FromObject(value);
        editCtx.modifiedPath = ctx.path;
    }

    return changed;
}

static bool EditObject_Internal_Fundamental_Bool(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    bool value = *reinterpret_cast<const bool*>(ctx.data);

    bool changed = ImGui::Checkbox("##value", &value);

    if (changed)
    {
        editCtx.objectAfter = Variant::FromObject(value);
        editCtx.modifiedPath = ctx.path;
    }

    return changed;
}

static bool EditObject_Internal_Fundamental(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    bool changed = false;

    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx("##name", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ctx.name);
    ImGui::NextColumn();

    // TODO value ranges

    // property value
    ImGui::SetNextItemWidth(-1);

    if (GetType<bool>() == ctx.type)            changed = EditObject_Internal_Fundamental_Bool(ctx, editCtx);
    else if (GetType<int8>() == ctx.type)       changed = EditObject_Internal_Fundamental_Typed<int8, ImGuiDataType_S8>(ctx, editCtx);
    else if (GetType<int16>() == ctx.type)      changed = EditObject_Internal_Fundamental_Typed<int16, ImGuiDataType_S16>(ctx, editCtx);
    else if (GetType<int32>() == ctx.type)      changed = EditObject_Internal_Fundamental_Typed<int32, ImGuiDataType_S32>(ctx, editCtx);
    else if (GetType<int64>() == ctx.type)      changed = EditObject_Internal_Fundamental_Typed<int64, ImGuiDataType_S64>(ctx, editCtx);
    else if (GetType<uint8>() == ctx.type)      changed = EditObject_Internal_Fundamental_Typed<uint8, ImGuiDataType_U8>(ctx, editCtx);
    else if (GetType<uint16>() == ctx.type)     changed = EditObject_Internal_Fundamental_Typed<uint16, ImGuiDataType_U16>(ctx, editCtx);
    else if (GetType<uint32>() == ctx.type)     changed = EditObject_Internal_Fundamental_Typed<uint32, ImGuiDataType_U32>(ctx, editCtx);
    else if (GetType<uint64>() == ctx.type)     changed = EditObject_Internal_Fundamental_Typed<uint64, ImGuiDataType_U64>(ctx, editCtx);
    else if (GetType<float>() == ctx.type)      changed = EditObject_Internal_Fundamental_Typed<float, ImGuiDataType_Float>(ctx, editCtx);
    else if (GetType<double>() == ctx.type)     changed = EditObject_Internal_Fundamental_Typed<double, ImGuiDataType_Float>(ctx, editCtx);
    else NFE_FATAL("Invalid fundamental type");

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_EnumItemGetter(void* data, int idx, const char** outText)
{
    const EnumOption* options = reinterpret_cast<const EnumOption*>(data);
    if (outText)
    {
        NFE_ASSERT(options[idx].name.IsNullTerminated(), "");
        *outText = options[idx].name.Data();
    }
    return true;
}

static bool EditObject_Internal_Enum(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    bool changed = false;

    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx(ctx.name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ctx.name);
    ImGui::NextColumn();

    // property value
    ImGui::SetNextItemWidth(-1);

    const EnumType* type = static_cast<const EnumType*>(ctx.type);
    const EnumOptions& options = type->GetOptions();

    const uint64 enumValue = type->ReadRawValue(ctx.data);

    int32 currentItem = -1;
    for (uint32 i = 0; i < options.Size(); ++i)
    {
        if (enumValue == options[i].value)
        {
            currentItem = (int32)i;
            break;
        }
    }

    changed = ImGui::Combo("##value", &currentItem, EditObject_EnumItemGetter, (void*)options.Data(), options.Size());

    if (changed && currentItem >= 0)
    {
        editCtx.objectAfter = std::move(type->MakeVariant(options[currentItem].value));
    }

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_Internal_Class_Members(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    const ClassType* type = static_cast<const ClassType*>(ctx.type);

    bool changed = false;

    Common::DynArray<Member> members;
    type->ListMembers(members);

    for (uint32 i = 0; i < members.Size(); ++i)
    {
        const Member& member = members[i];

        const void* memberData = reinterpret_cast<const uint8*>(ctx.data) + member.GetOffset();

        MemberPath childPath = ctx.path;
        childPath.Append(member.GetName());

        struct EditPropertyContext propertyContext =
        {
            childPath,
            member.GetDescriptiveName(),
            member.GetType(),
            memberData,
            &(member.GetMetadata()),
        };

        ImGui::PushID(i);
        const bool propertyChanged = EditObject_Internal(propertyContext, editCtx);
        ImGui::PopID();

        if (propertyChanged)
        {
            //// notify object
            //if (type->IsA(GetType<IObject>()))
            //{
            //    IObject* object = reinterpret_cast<IObject*>(data);
            //    const bool propertyChangeHandled = object->OnPropertyChanged(StringView(member.GetName()));
            //    NFE_ASSERT(propertyChangeHandled, "Property change not handled");
            //}
            changed = true;
        }
    }

    return changed;
}

static bool EditObject_Internal_Class(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    const ClassType* type = static_cast<const ClassType*>(ctx.type);

    bool changed = false;

    ImGui::AlignTextToFramePadding();
    bool isOpen = ImGui::TreeNode(ctx.name, ctx.name);
    ImGui::NextColumn();

    // class type name on right column
    ImGui::TextDisabled("(%s)", type->GetName().Str());
    ImGui::NextColumn();

    if (isOpen)
    {
        changed = EditObject_Internal_Class_Members(ctx, editCtx);
        ImGui::TreePop();
    }

    return changed;
}

static bool EditObject_TypeGetter(void* data, int idx, const char** outText)
{
    const Type** types = reinterpret_cast<const Type**>(data);
    if (outText)
    {
        const Type* type = types[idx];

        if (type)
        {
            *outText = type->GetName().Str();
        }
        else
        {
            *outText = "<Empty>";
        }
    }
    return true;
}

static bool EditObject_Internal_Pointer(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    bool changed = false;

    const PointerType* type = static_cast<const PointerType*>(ctx.type);

    // property name
    ImGui::AlignTextToFramePadding();
    bool isOpen = ImGui::TreeNode(ctx.name, ctx.name);
    ImGui::NextColumn();

    // property value
    ImGui::SetNextItemWidth(-1);

    // fill up the list of types that can be set by this unique ptr
    Common::DynArray<const Type*> typesList;

    bool nonNull = ctx.metadata && ctx.metadata->nonNull;
    if (!nonNull)
    {
        typesList.PushBack(nullptr);
    }

    const Type* pointedType = type->GetUnderlyingType();
    if (pointedType->GetKind() == TypeKind::Class)
    {
        static_cast<const ClassType*>(pointedType)->ListSubtypes([&typesList] (const ClassType* type)
        {
            if (type->IsConstructible())
            {
                typesList.PushBack(type);
            }
        }, /*skipAbstractTypes*/ true);
    }
    else
    {
        typesList.PushBack(pointedType);
    }

    int32 currentItem = -1;
    for (uint32 i = 0; i < typesList.Size(); ++i)
    {
        if (typesList[i] == type->GetPointedDataType(ctx.data))
        {
            currentItem = i;
        }
    }

    changed = ImGui::Combo("##value", &currentItem, EditObject_TypeGetter, typesList.Data(), typesList.Size());

    const Type* currentType = currentItem >= 0 ? typesList[currentItem] : nullptr;

    if (changed)
    {
        // TODO create variant
        //type->Reset(ctx.data, currentType);
        NFE_FATAL("Not implemented");
    }

    ImGui::NextColumn();

    if (isOpen)
    {
        if (currentType)
        {
            struct EditPropertyContext propertyContext =
            {
                ctx.path,
                "[value]",
                currentType,
                type->GetPointedData(ctx.data),
            };

            if (currentType->IsA(GetType<IObject>()))
            {
                // skip header for class types
                changed |= EditObject_Internal_Class_Members(propertyContext, editCtx);
            }
            else
            {
                changed |= EditObject_Internal(propertyContext, editCtx);
            }
        }
        ImGui::TreePop();
    }

    return changed;
}

static bool EditObject_Internal_NativeArray(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    bool changed = false;

    const NativeArrayType* type = static_cast<const NativeArrayType*>(ctx.type);

    ImGui::AlignTextToFramePadding();
    bool isOpen = ImGui::TreeNode(ctx.name, ctx.name);
    ImGui::NextColumn();

    // class type name on right column
    ImGui::TextDisabled("%u-element array", type->GetArraySize());
    ImGui::NextColumn();

    if (isOpen)
    {
        const uint32 size = type->GetArraySize();

        for (uint32 index = 0; index < size; ++index)
        {
            const void* elementData = type->GetElementPointer(ctx.data, index);

            char elementIndexStr[16];
            sprintf(elementIndexStr, "[%u]", index);

            MemberPath childPath = ctx.path;
            childPath.Append(index);

            struct EditPropertyContext propertyContext =
            {
                childPath,
                elementIndexStr,
                type->GetUnderlyingType(),
                elementData,
            };

            ImGui::PushID(index);
            changed |= EditObject_Internal(propertyContext, editCtx);
            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    return changed;
}

static bool EditObject_Internal_DynArray(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    bool changed = false;

    const DynArrayType* type = static_cast<const DynArrayType*>(ctx.type);

    ImGui::AlignTextToFramePadding();
    bool isOpen = ImGui::TreeNode(ctx.name, ctx.name);
    ImGui::NextColumn();

    const uint32 arraySize = type->GetArraySize(ctx.data);

    // class type name on right column
    ImGui::TextDisabled("%u-element array", arraySize);
    ImGui::NextColumn();

    if (isOpen)
    {
        for (uint32 index = 0; index < arraySize; ++index)
        {
            const void* elementData = type->GetElementPointer(ctx.data, index);

            char elementIndexStr[16];
            sprintf(elementIndexStr, "[%u]", index);

            MemberPath childPath = ctx.path;
            childPath.Append(index);

            struct EditPropertyContext propertyContext =
            {
                childPath,
                elementIndexStr,
                type->GetUnderlyingType(),
                elementData,
            };

            ImGui::PushID(index);
            changed |= EditObject_Internal(propertyContext, editCtx);
            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    return changed;
}

static bool EditObject_Internal(const EditPropertyContext& ctx, EditObjectContext& editCtx)
{
    bool changed = false;

    if (ctx.type == RTTI::GetType<Math::LdrColorRGB>())
    {
        changed = EditObject_Internal_LdrColorRGB(ctx.name, static_cast<const Math::LdrColorRGB*>(ctx.data), editCtx);
    }
    else if (ctx.type == RTTI::GetType<Math::HdrColorRGB>())
    {
        changed = EditObject_Internal_HdrColorRGB(ctx.name, static_cast<const Math::HdrColorRGB*>(ctx.data), editCtx);
    }
    else if (ctx.type == RTTI::GetType<Math::Vec2f>())
    {
        changed = EditObject_Internal_FloatVector(ctx.name, static_cast<const float*>(ctx.data), 2, editCtx);
    }
    else if (ctx.type == RTTI::GetType<Math::Vec3f>())
    {
        changed = EditObject_Internal_FloatVector(ctx.name, static_cast<const float*>(ctx.data), 3, editCtx);
    }
    else if (ctx.type == RTTI::GetType<Math::Vec4fU>())
    {
        changed = EditObject_Internal_FloatVector(ctx.name, static_cast<const float*>(ctx.data), 4, editCtx);
    }
    else if (ctx.type == RTTI::GetType<Math::Vec4f>())
    {
        changed = EditObject_Internal_FloatVector(ctx.name, static_cast<const float*>(ctx.data), 4, editCtx);
    }
    else if (ctx.type == RTTI::GetType<Math::Quaternion>())
    {
        changed = EditObject_Internal_Quaternion(ctx.name, static_cast<const Math::Quaternion*>(ctx.data), editCtx);
    }
    else
    {
        switch (ctx.type->GetKind())
        {
        case TypeKind::Fundamental:
            changed = EditObject_Internal_Fundamental(ctx, editCtx);
            break;
        case TypeKind::Enumeration:
            changed = EditObject_Internal_Enum(ctx, editCtx);
            break;
        case TypeKind::Class:
            changed = EditObject_Internal_Class(ctx, editCtx);
            break;
        case TypeKind::UniquePtr:
        case TypeKind::SharedPtr:
            changed = EditObject_Internal_Pointer(ctx, editCtx);
            break;
        case TypeKind::NativeArray:
            changed = EditObject_Internal_NativeArray(ctx, editCtx);
            break;
        case TypeKind::DynArray:
            changed = EditObject_Internal_DynArray(ctx, editCtx);
            break;
        default:
            NFE_FATAL("Invalid type");
            break;
        }
    }

    return changed;
}

bool EditObject_Root_Internal(const char* rootName, const Type* type, const void* data, EditObjectContext& editCtx)
{
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
    ImGui::Columns(2);

    struct EditPropertyContext propertyContext = { MemberPath(), rootName, type, data };

    bool changed = EditObject_Internal(propertyContext, editCtx);

    ImGui::Columns(1);
    ImGui::PopStyleVar();

    return changed;
}

bool ApplyObjectChanges_Internal(const RTTI::Type* type, void* data, const EditObjectContext& context)
{
    const RTTI::Type* memberType = nullptr;
    void* memberData = nullptr;

    if (!type->GetMemberByPath(data, context.modifiedPath, memberType, memberData))
    {
        NFE_LOG_ERROR("Failed to find edited member");
        return false;
    }

    NFE_ASSERT(memberData, "Invalid member data");
    NFE_ASSERT(memberType == context.objectAfter.GetType(),
        "Member type (%s) does not match type found in variant (%s)",
        memberType->GetName().Str(), context.objectAfter.GetType()->GetName().Str());

    if (!memberType->Clone(memberData, context.objectAfter.GetData()))
    {
        NFE_LOG_ERROR("Failed to clone object data");
        return false;
    }

    return true;
}

} // namesapce NFE