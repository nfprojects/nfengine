/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config value declaration.
 */

#pragma once

#include "ConfigCommon.hpp"


namespace NFE {
namespace Common {

class Config;

/**
 * Class representing a value in the config tree. A value can be of simple type
 * (bool, int, float, string), subobject or an array of values.
 */
class NFCOMMON_API ConfigValue
{
    friend class Config;

protected:
    enum class Type
    {
        None,
        Bool,
        Int8, Uint8,
        Int16, Uint16,
        Int32, Uint32,
        Int64, Uint64,
        Float, Double,
        String,
        Object,
        Array,
    };

    union
    {
        bool boolData;
        int8 intData8;
        int16 intData16;
        int32 intData32;
        int64 intData64;
        uint8 uintData8;
        uint16 uintData16;
        uint32 uintData32;
        uint64 uintData64;
        float floatData;
        double doubleData;
        const char* stringData;
        ConfigObjectNodePtr object;
        ConfigArrayNodePtr array;
    };

    Type type;

public:
    ConfigValue() : object(INVALID_NODE_PTR), type(Type::None) { }

    /// Constructors
    ConfigValue(bool val) { type = Type::Bool; boolData = val; }
    explicit ConfigValue(int8 val) { type = Type::Int8; intData8 = val; }
    explicit ConfigValue(uint8 val) { type = Type::Uint8; uintData8 = val; }
    explicit ConfigValue(int16 val) { type = Type::Int16; intData16 = val; }
    explicit ConfigValue(uint16 val) { type = Type::Uint16; uintData16 = val; }
    explicit ConfigValue(int32 val) { type = Type::Int32; intData32 = val; }
    explicit ConfigValue(uint32 val) { type = Type::Uint32; uintData32 = val; }
    explicit ConfigValue(int64 val) { type = Type::Int64; intData64 = val; }
    explicit ConfigValue(uint64 val) { type = Type::Uint64; uintData64 = val; }
    ConfigValue(float val) { type = Type::Float; floatData = val; }
    ConfigValue(double val) { type = Type::Double; doubleData = val; }
    ConfigValue(const char* val) { type = Type::String; stringData = val; }
    ConfigValue(const ConfigObject& val) { type = Type::Object; object = val.mHead; }
    ConfigValue(const ConfigArray& val) { type = Type::Array; array = val.mHead; }

    /// Checkers
    template <typename T>
    NFE_INLINE bool Is() const;
    NFE_INLINE bool IsString() const { return type == Type::String; }
    NFE_INLINE bool IsObject() const { return type == Type::Object; }
    NFE_INLINE bool IsArray() const { return type == Type::Array; }

    /// Getters
    template <typename T>
    NFE_INLINE T Get() const;
    NFE_INLINE const char* GetString() const { return stringData; }
    NFE_INLINE ConfigObjectNodePtr GetObj() const { return object; }
    NFE_INLINE ConfigArrayNodePtr GetArray() const { return array; }
};

// ConfigValue::Is<T>() template specializations
template<> NFE_INLINE bool ConfigValue::Is<int8>() const { return type == Type::Int8; }
template<> NFE_INLINE bool ConfigValue::Is<uint8>() const { return type == Type::Uint8; }
template<> NFE_INLINE bool ConfigValue::Is<int16>() const { return type == Type::Int16; }
template<> NFE_INLINE bool ConfigValue::Is<uint16>() const { return type == Type::Uint16; }
template<> NFE_INLINE bool ConfigValue::Is<int32>() const { return type == Type::Int32; }
template<> NFE_INLINE bool ConfigValue::Is<uint32>() const { return type == Type::Uint32; }
template<> NFE_INLINE bool ConfigValue::Is<int64>() const { return type == Type::Int64; }
template<> NFE_INLINE bool ConfigValue::Is<uint64>() const { return type == Type::Uint64; }
template<> NFE_INLINE bool ConfigValue::Is<bool>() const { return type == Type::Bool; }
template<> NFE_INLINE bool ConfigValue::Is<float>() const { return type == Type::Float; }
template<> NFE_INLINE bool ConfigValue::Is<double>() const { return type == Type::Double; }
template<> NFE_INLINE bool ConfigValue::Is<const char*>() const { return type == Type::String; }

// Get<T>() template specializations
template<> NFE_INLINE int8 ConfigValue::Get<int8>() const { return intData8; }
template<> NFE_INLINE uint8 ConfigValue::Get<uint8>() const { return uintData8; }
template<> NFE_INLINE int16 ConfigValue::Get<int16>() const { return intData16; }
template<> NFE_INLINE uint16 ConfigValue::Get<uint16>() const { return uintData16; }
template<> NFE_INLINE int32 ConfigValue::Get<int32>() const { return intData32; }
template<> NFE_INLINE uint32 ConfigValue::Get<uint32>() const { return uintData32; }
template<> NFE_INLINE int64 ConfigValue::Get<int64>() const { return intData64; }
template<> NFE_INLINE uint64 ConfigValue::Get<uint64>() const { return uintData64; }
template<> NFE_INLINE bool ConfigValue::Get<bool>() const { return boolData; }
template<> NFE_INLINE float ConfigValue::Get<float>() const { return floatData; }
template<> NFE_INLINE double ConfigValue::Get<double>() const { return doubleData; }
template<> NFE_INLINE const char* ConfigValue::Get<const char*>() const { return stringData; }

//////////////////////////////////////////////////////////////////////////

/**
 * Extension of ConfigValue allowing for accessing child values
 * (sub-objects or array elements) with easy-to-use operators at expense of performance.
 */
class NFCOMMON_API ConfigGenericValue : public ConfigValue
{
    const Config* mConfig;

    // TODO: implement a map of object keys / array indicies to speed up lookup

public:
    ConfigGenericValue() : ConfigValue(), mConfig(nullptr) { }
    ConfigGenericValue(const Config* config);
    ConfigGenericValue(const Config* config, const ConfigValue& val);

    /**
     * Check if a value with a given key exists in the object.
     */
    bool HasMember(StringView key) const;
    bool HasMember(const char* key) const
    {
        return HasMember(StringView(key));
    }

    /**
     * Find value in object by key.
     */
    ConfigGenericValue operator[](StringView key) const;
    ConfigGenericValue operator[](const char* key) const
    {
        return operator[](StringView(key));
    }

    /**
     * Get array size.
     */
    uint32 GetSize() const;

    /**
     * Get value from array by index.
     */
    ConfigGenericValue operator[](uint32 index) const;
};


} // namespace Common
} // namespace NFE
