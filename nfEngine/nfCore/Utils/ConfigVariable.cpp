/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config variables definitions
 */

#include "../PCH.hpp"
#include "ConfigVariable.hpp"

#include "nfCommon/Config/Config.hpp"
#include "nfCommon/FileSystem/File.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Math/Math.hpp"

namespace NFE {

bool IConfigVariable::ParseConfigValue(const Common::ConfigValue& value)
{
    if (value.IsInt32() && mType == ConfigVarType::Integer)
    {
        auto var = static_cast<ConfigVariable<int>*>(this);
        var->mValue = value.GetInt32();
        return true;
    }
    else if (value.IsBool() && mType == ConfigVarType::Bool)
    {
        auto var = static_cast<ConfigVariable<bool>*>(this);
        var->mValue = value.GetBool();
        return true;
    }
    else if (value.IsString() && mType == ConfigVarType::String)
    {
        auto var = static_cast<ConfigVariable<const char*>*>(this);
        var->mValue = value.GetString();
        return true;
    }
    else if (value.IsFloat() && mType == ConfigVarType::Float)
    {
        auto var = static_cast<ConfigVariable<float>*>(this);
        var->mValue = value.GetFloat();
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
void ConfigVariable<Math::Float2>::SetType()
{
    mType = ConfigVarType::Float2;
}

template<>
void ConfigVariable<Math::Float3>::SetType()
{
    mType = ConfigVarType::Float3;
}

template<>
void ConfigVariable<Math::Float4>::SetType()
{
    mType = ConfigVarType::Float4;
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
std::string ConfigVariable<Math::Float2>::ToString() const
{
    return '[' + std::to_string(mValue.x) + ", " + std::to_string(mValue.y) + ']';
}

template<>
std::string ConfigVariable<Math::Float3>::ToString() const
{
    return '[' + std::to_string(mValue.x) + ", " + std::to_string(mValue.y) + ", " + std::to_string(mValue.z) + ']';
}

template<>
std::string ConfigVariable<Math::Float4>::ToString() const
{
    return '[' + std::to_string(mValue.x) + ", " + std::to_string(mValue.y) + ", " + std::to_string(mValue.z) + ", " + std::to_string(mValue.w) + ']';
}

} // namespace NFE
