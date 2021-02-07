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

namespace NFE {

using namespace NFE::RTTI;
using namespace NFE::Common;

struct EditPropertyContext
{
    const char* name;
    const Type* type;
    void* data;
    const MemberMetadata* metadata = nullptr;
};

static bool EditObject_Internal(const EditPropertyContext& ctx);

static bool EditObject_Internal_LdrColorRGB(const char* name, Math::LdrColorRGB* color)
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
        color->r = static_cast<uint8>(values[0] * 255.0f);
        color->g = static_cast<uint8>(values[1] * 255.0f);
        color->b = static_cast<uint8>(values[2] * 255.0f);
        changed = true;
    }

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_Internal_HdrColorRGB(const char* name, Math::HdrColorRGB* color)
{
    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, name);
    ImGui::NextColumn();

    // property value
    ImGui::SetNextItemWidth(-1);

	bool changed = ImGui::ColorEdit3("", &(color->r), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    color->r = Math::Max(0.0f, color->r);
    color->g = Math::Max(0.0f, color->g);
    color->b = Math::Max(0.0f, color->b);

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_Internal_FloatVector(const char* name, float* data, uint32 numComponents)
{
    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, name);
    ImGui::NextColumn();

    // property value
    ImGui::SetNextItemWidth(-1);

    bool changed = ImGui::InputScalarN("", ImGuiDataType_Float, data, numComponents, nullptr, nullptr, "%.4f");

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_Internal_Quaternion(const char* name, Math::Quaternion* quat)
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
        *quat = Math::Quaternion::FromEulerAngles(angles);
        changed = true;
    }

    ImGui::NextColumn();

    return changed;
}

template<typename T, ImGuiDataType_ ImGuiDataType>
static bool EditObject_Internal_Fundamental_Typed(const EditPropertyContext& ctx)
{
    const T min = (ctx.metadata && ctx.metadata->HasMinRange()) ? static_cast<T>(ctx.metadata->min) : std::numeric_limits<T>::min();
    const T max = (ctx.metadata && ctx.metadata->HasMaxRange()) ? static_cast<T>(ctx.metadata->max) : std::numeric_limits<T>::max();

    const bool hasSlider = ctx.metadata && !ctx.metadata->HasNoSlider() && ctx.metadata->HasMinRange() && ctx.metadata->HasMaxRange();

    bool changed = false;

    if (!hasSlider)
    {
        changed = ImGui::InputScalar("##value", ImGuiDataType, reinterpret_cast<T*>(ctx.data));
    }
    else
    {
        const float power = std::is_floating_point_v<T> ? ctx.metadata->logScalePower : 1.0f;

        changed = ImGui::SliderScalar("##value", ImGuiDataType, reinterpret_cast<T*>(ctx.data), &min, &max, nullptr, power);
    }

    return changed;
}

static bool EditObject_Internal_Fundamental(const EditPropertyContext& ctx)
{
    bool changed = false;

    // property name
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx("##name", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ctx.name);
    ImGui::NextColumn();

    // TODO value ranges

    // property value
    ImGui::SetNextItemWidth(-1);
    if (GetType<bool>() == ctx.type)
    {
        changed = ImGui::Checkbox("##value", reinterpret_cast<bool*>(ctx.data));
    }
    else if (GetType<int8>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<int8, ImGuiDataType_S8>(ctx);
    }
    else if (GetType<int16>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<int16, ImGuiDataType_S16>(ctx);
    }
    else if (GetType<int32>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<int32, ImGuiDataType_S32>(ctx);
    }
    else if (GetType<int64>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<int64, ImGuiDataType_S64>(ctx);
    }
    else if (GetType<uint8>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<uint64, ImGuiDataType_U8>(ctx);
    }
    else if (GetType<uint16>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<uint64, ImGuiDataType_U16>(ctx);
    }
    else if (GetType<uint32>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<uint64, ImGuiDataType_U32>(ctx);
    }
    else if (GetType<uint64>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<uint64, ImGuiDataType_U64>(ctx);
    }
    else if (GetType<float>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<float, ImGuiDataType_Float>(ctx);
    }
    else if (GetType<double>() == ctx.type)
    {
        changed = EditObject_Internal_Fundamental_Typed<double, ImGuiDataType_Float>(ctx);
    }
    else
    {
        NFE_FATAL("Invalid fundamental type");
    }

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

static bool EditObject_Internal_Enum(const EditPropertyContext& ctx)
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
        type->WriteRawValue(ctx.data, options[currentItem].value);
    }

    ImGui::NextColumn();

    return changed;
}

static bool EditObject_Internal_Class_Members(const ClassType* type, void* data)
{
    bool changed = false;

    Common::DynArray<Member> members;
    type->ListMembers(members);

    for (uint32 i = 0; i < members.Size(); ++i)
    {
        const Member& member = members[i];

        void* memberData = reinterpret_cast<uint8*>(data) + member.GetOffset();

        struct EditPropertyContext propertyContext =
        {
            member.GetDescriptiveName(),
            member.GetType(),
            memberData,
            &(member.GetMetadata()),
        };

        ImGui::PushID(i);
        const bool propertyChanged = EditObject_Internal(propertyContext);
        ImGui::PopID();

        if (propertyChanged)
        {
            // notify object
            if (type->IsA(GetType<IObject>()))
            {
                IObject* object = reinterpret_cast<IObject*>(data);
                const bool propertyChangeHandled = object->OnPropertyChanged(StringView(member.GetName()));
                NFE_ASSERT(propertyChangeHandled, "Property change not handled");
            }
            changed = true;
        }
    }

    return changed;
}

static bool EditObject_Internal_Class(const EditPropertyContext& ctx)
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
        changed = EditObject_Internal_Class_Members(type, ctx.data);
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

static bool EditObject_Internal_Pointer(const EditPropertyContext& ctx)
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
        type->Reset(ctx.data, currentType);
    }

    ImGui::NextColumn();

    if (isOpen)
    {
        if (currentType)
        {
            if (currentType->IsA(GetType<IObject>()))
            {
                // skip header for class types
                changed |= EditObject_Internal_Class_Members(static_cast<const ClassType*>(currentType), type->GetPointedData(ctx.data));
            }
            else
            {
                struct EditPropertyContext propertyContext =
                {
                    "[value]",
                    currentType,
                    type->GetPointedData(ctx.data),
                };

                changed |= EditObject_Internal(propertyContext);
            }
        }
        ImGui::TreePop();
    }

    return changed;
}

static bool EditObject_Internal_NativeArray(const EditPropertyContext& ctx)
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

        for (uint32 i = 0; i < size; ++i)
        {
            void* elementData = type->GetElementPointer(ctx.data, i);

            char elementIndexStr[16];
            sprintf(elementIndexStr, "[%u]", i);

            struct EditPropertyContext propertyContext =
            {
                elementIndexStr,
                type->GetUnderlyingType(),
                elementData,
            };

            ImGui::PushID(i);
            changed |= EditObject_Internal(propertyContext);
            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    return changed;
}

static bool EditObject_Internal_Array(const EditPropertyContext& ctx)
{
    bool changed = false;

    const ResizableArrayType* type = static_cast<const ResizableArrayType*>(ctx.type);

    ImGui::AlignTextToFramePadding();
    bool isOpen = ImGui::TreeNode(ctx.name, ctx.name);
    ImGui::NextColumn();

    const uint32 arraySize = type->GetArraySize(ctx.data);

    // class type name on right column
    ImGui::TextDisabled("%u-element array", arraySize);
    ImGui::NextColumn();

    if (isOpen)
    {
        for (uint32 i = 0; i < arraySize; ++i)
        {
            void* elementData = type->GetElementPointer(ctx.data, i);

            char elementIndexStr[16];
            sprintf(elementIndexStr, "[%u]", i);

            struct EditPropertyContext propertyContext =
            {
                elementIndexStr,
                type->GetUnderlyingType(),
                elementData,
            };

            ImGui::PushID(i);
            changed |= EditObject_Internal(propertyContext);
            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    return changed;
}

static bool EditObject_Internal(const EditPropertyContext& ctx)
{
    bool changed = false;

    if (ctx.type == RTTI::GetType<Math::LdrColorRGB>())
    {
        changed = EditObject_Internal_LdrColorRGB(ctx.name, static_cast<Math::LdrColorRGB*>(ctx.data));
    }
    else if (ctx.type == RTTI::GetType<Math::HdrColorRGB>())
    {
        changed = EditObject_Internal_HdrColorRGB(ctx.name, static_cast<Math::HdrColorRGB*>(ctx.data));
    }
    else if (ctx.type == RTTI::GetType<Math::Vec2f>())
    {
        changed = EditObject_Internal_FloatVector(ctx.name, static_cast<float*>(ctx.data), 2);
    }
    else if (ctx.type == RTTI::GetType<Math::Vec3f>())
    {
        changed = EditObject_Internal_FloatVector(ctx.name, static_cast<float*>(ctx.data), 3);
    }
    else if (ctx.type == RTTI::GetType<Math::Vec4fU>())
    {
        changed = EditObject_Internal_FloatVector(ctx.name, static_cast<float*>(ctx.data), 4);
    }
    else if (ctx.type == RTTI::GetType<Math::Vec4f>())
    {
        changed = EditObject_Internal_FloatVector(ctx.name, static_cast<float*>(ctx.data), 4);
    }
    else if (ctx.type == RTTI::GetType<Math::Quaternion>())
    {
        changed = EditObject_Internal_Quaternion(ctx.name, static_cast<Math::Quaternion*>(ctx.data));
    }
    else
    {
        switch (ctx.type->GetKind())
        {
        case TypeKind::Fundamental:
            changed = EditObject_Internal_Fundamental(ctx);
            break;
        case TypeKind::Enumeration:
            changed = EditObject_Internal_Enum(ctx);
            break;
        case TypeKind::Class:
            changed = EditObject_Internal_Class(ctx);
            break;
        case TypeKind::UniquePtr:
        case TypeKind::SharedPtr:
            changed = EditObject_Internal_Pointer(ctx);
            break;
        case TypeKind::NativeArray:
            changed = EditObject_Internal_NativeArray(ctx);
            break;
        case TypeKind::Array:
            changed = EditObject_Internal_Array(ctx);
            break;
        default:
            NFE_FATAL("Invalid type");
            break;
        }
    }

    return changed;
}

bool EditObject_Root_Internal(const char* rootName, const Type* type, void* data)
{
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
    ImGui::Columns(2);

    struct EditPropertyContext propertyContext = { rootName, type, data, };

    bool changed = EditObject_Internal(propertyContext);

    ImGui::Columns(1);
    ImGui::PopStyleVar();

    return changed;
}

} // namesapce NFE