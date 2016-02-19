/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once
#include "Config.hpp"

namespace NFE {
namespace Common {

struct CStringComparator
{
    bool operator() (const char* a, const char* b) const
    {
        return strcmp(a, b) < 0;
    }
};

/**
 * Helper structure for easy structures parsing (without need to iterating through
 * config nodes).
 */
template <class T>
class DataTranslator
{
    friend class Config;

    // single values
    typedef bool T::*BoolValue;
    typedef int T::*IntValue;
    typedef float T::*FloatValue;
    typedef const char* T::*StringValue;

    // arrays of values
    typedef std::vector<bool> T::*BoolArrayValue;
    typedef std::vector<int> T::*IntArrayValue;
    typedef std::vector<float> T::*FloatArrayValue;
    typedef std::vector<const char*> T::*StringArrayValue;

    std::map<const char*, BoolValue, CStringComparator> mBoolValues;
    std::map<const char*, IntValue, CStringComparator> mIntValues;
    std::map<const char*, FloatValue, CStringComparator> mFloatValues;
    std::map<const char*, StringValue, CStringComparator> mStringValues;
    std::map<const char*, BoolArrayValue, CStringComparator> mBoolArrayValues;
    std::map<const char*, IntArrayValue, CStringComparator> mIntArrayValues;
    std::map<const char*, FloatArrayValue, CStringComparator> mFloatArrayValues;
    std::map<const char*, StringArrayValue, CStringComparator> mStringArrayValues;

public:
    DataTranslator& Add(const char* name, BoolValue value)
    {
        mBoolValues[name] = value;
        return *this;
    }

    DataTranslator& Add(const char* name, IntValue value)
    {
        mIntValues[name] = value;
        return *this;
    }

    DataTranslator& Add(const char* name, FloatValue value)
    {
        mFloatValues[name] = value;
        return *this;
    }

    DataTranslator& Add(const char* name, StringValue value)
    {
        mStringValues[name] = value;
        return *this;
    }

    DataTranslator& Add(const char* name, BoolArrayValue value)
    {
        mBoolArrayValues[name] = value;
        return *this;
    }

    DataTranslator& Add(const char* name, IntArrayValue value)
    {
        mIntArrayValues[name] = value;
        return *this;
    }

    DataTranslator& Add(const char* name, FloatArrayValue value)
    {
        mFloatArrayValues[name] = value;
        return *this;
    }

    DataTranslator& Add(const char* name, StringArrayValue value)
    {
        mStringArrayValues[name] = value;
        return *this;
    }

    // TODO:
    // * nested objects translating
    // * arrays
};

template <typename T>
bool Config::TranslateConfigObject(ConfigObjectNodePtr node,
                                   DataTranslator<T>& translator,
                                   T& object) const
{
    auto iteratorCallback = [&](const char* key, const ConfigValue& value)
    {
        if (translator.mBoolValues.count(key) > 0)
        {
            auto memberPtr = translator.mBoolValues[key];
            object.*memberPtr = value.GetBool();
        }
        else if (translator.mIntValues.count(key) > 0)
        {
            auto memberPtr = translator.mIntValues[key];
            object.*memberPtr = value.GetInt();
        }
        else if (translator.mFloatValues.count(key) > 0)
        {
            auto memberPtr = translator.mFloatValues[key];
            object.*memberPtr = value.GetFloat();
        }
        else if (translator.mStringValues.count(key) > 0)
        {
            auto memberPtr = translator.mStringValues[key];
            object.*memberPtr = value.GetString();
        }
        return true;
    };

    Iterate(iteratorCallback, node);

    return true;
}

} // namespace Common
} // namespace NFE
