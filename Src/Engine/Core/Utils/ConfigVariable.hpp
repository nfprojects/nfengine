/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config variables declarations
 */

#pragma once

#include "../Core.hpp"
#include "ConfigManager.hpp"
#include "../../Common/nfCommon.hpp"


namespace NFE {

// predeclaration
namespace Common {
class Config;
class ConfigValue;
}


enum class ConfigVarType
{
    Integer,
    Float,
    Float2,
    Float3,
    Float4,
    Bool,
    String,
};

class IConfigVariable
{
protected:
    const char* mPath;
    ConfigVarType mType;

public:
    IConfigVariable(const char* path)
        : mPath(path)
    { }

    NFE_INLINE virtual ~IConfigVariable() { }
    NFE_INLINE const char* GetPath() const { return mPath; }
    NFE_INLINE ConfigVarType GetType() const { return mType; }

    /**
     * Read value from nfCommon's config value.
     */
    bool ParseConfigValue(const Common::ConfigValue& value);

    /**
     * Convert config variable value to string.
     */
    virtual std::string ToString() const = 0;
};


/**
 * Config variable helper class.
 * Create a global instance of this class to register a engine's config variable.
 */
template<typename T>
class ConfigVariable : public IConfigVariable
{
    friend class IConfigVariable;
    NFE_MAKE_NONCOPYABLE(ConfigVariable)
    NFE_MAKE_NONMOVEABLE(ConfigVariable)

private:
    T mDefaultValue;
    T mValue;

    void SetType();

public:
    ConfigVariable(const char* path, const T& defaultValue)
        : IConfigVariable(path)
        , mDefaultValue(defaultValue)
        , mValue(defaultValue)
    {
        SetType();
        ConfigManager::GetInstance().RegisterVariable(this);
    }

    NFE_INLINE const T& Get() const
    {
        return mValue;
    }

    NFE_INLINE void Set(const T& val)
    {
        mValue = val;
    }

    std::string ToString() const override;
};

} // namespace NFE
