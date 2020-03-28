/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config variables definitions
 */

#include "../PCH.hpp"
#include "ConfigVariable.hpp"

#include "Engine/Common/Config/Config.hpp"
#include "Engine/Common/Math/Vec4fU.hpp"


namespace NFE {

bool IConfigVariable::ParseConfigValue(const Common::ConfigValue& value)
{
    if (value.Is<int32>() && mType == ConfigVarType::Integer)
    {
        auto var = static_cast<ConfigVariable<int>*>(this);
        var->mValue = value.Get<int32>();
        return true;
    }
    else if (value.Is<bool>() && mType == ConfigVarType::Bool)
    {
        auto var = static_cast<ConfigVariable<bool>*>(this);
        var->mValue = value.Get<bool>();
        return true;
    }
    else if (value.IsString() && mType == ConfigVarType::String)
    {
        auto var = static_cast<ConfigVariable<const char*>*>(this);
        var->mValue = value.GetString();
        return true;
    }
    else if (value.Is<float>() && mType == ConfigVarType::Float)
    {
        auto var = static_cast<ConfigVariable<float>*>(this);
        var->mValue = value.Get<float>();
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

template<>
void ConfigVariable<int>::SetType()
{
    mType = ConfigVarType::Integer;
}

template<>
void ConfigVariable<bool>::SetType()
{
    mType = ConfigVarType::Bool;
}

template<>
void ConfigVariable<const char*>::SetType()
{
    mType = ConfigVarType::String;
}

template<>
void ConfigVariable<float>::SetType()
{
    mType = ConfigVarType::Float;
}

template<>
void ConfigVariable<Math::Vec2f>::SetType()
{
    mType = ConfigVarType::Vec2f;
}

template<>
void ConfigVariable<Math::Vec3f>::SetType()
{
    mType = ConfigVarType::Vec3f;
}

template<>
void ConfigVariable<Math::Vec4fU>::SetType()
{
    mType = ConfigVarType::Vec4fU;
}


template<>
std::string ConfigVariable<bool>::ToString() const
{
    return mValue ? "true" : "false";
}

template<>
std::string ConfigVariable<int>::ToString() const
{
    return std::to_string(mValue);
}

template<>
std::string ConfigVariable<float>::ToString() const
{
    return std::to_string(mValue);
}

template<>
std::string ConfigVariable<const char*>::ToString() const
{
    return '\"' + std::string(mValue) + '\"';
}

template<>
std::string ConfigVariable<Math::Vec2f>::ToString() const
{
    return '[' + std::to_string(mValue.x) + ", " + std::to_string(mValue.y) + ']';
}

template<>
std::string ConfigVariable<Math::Vec3f>::ToString() const
{
    return '[' + std::to_string(mValue.x) + ", " + std::to_string(mValue.y) + ", " + std::to_string(mValue.z) + ']';
}

template<>
std::string ConfigVariable<Math::Vec4fU>::ToString() const
{
    return '[' + std::to_string(mValue.x) + ", " + std::to_string(mValue.y) + ", " + std::to_string(mValue.z) + ", " + std::to_string(mValue.w) + ']';
}

} // namespace NFE
