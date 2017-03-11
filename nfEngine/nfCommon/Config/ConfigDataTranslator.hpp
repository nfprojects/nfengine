/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once

#include "Config.hpp"
#include "../Logger/Logger.hpp"


namespace NFE {
namespace Common {

struct CStringComparator
{
    bool operator() (const char* a, const char* b) const
    {
        return strcmp(a, b) < 0;
    }
};

// TODO support for parsing nested objects (not trivial)

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
};


template <typename T>
bool Config::TranslateConfigObject(ConfigObjectNodePtr node,
                                   DataTranslator<T>& translator,
                                   T& object) const
{
    using namespace std::placeholders;

    bool success = true;

    auto intArrayIterator = [&success](const char* key, const ConfigValue& value,
                                       std::vector<int>& array)
    {
        if (!value.IsInt())
        {
            success = false;
            LOG_ERROR("Array '%s' contains non-integer value", key);
            return false;
        }
        array.push_back(value.GetInt());
        return true;
    };

    auto boolArrayIterator = [&success](const char* key, const ConfigValue& value,
                                        std::vector<bool>& array)
    {
        if (!value.IsBool())
        {
            success = false;
            LOG_ERROR("Array '%s' contains non-boolean value", key);
            return false;
        }
        array.push_back(value.GetBool());
        return true;
    };

    auto floatArrayIterator = [&success](const char* key, const ConfigValue& value,
                                         std::vector<float>& array)
    {
        if (!value.IsFloat())
        {
            success = false;
            LOG_ERROR("Array '%s' contains non-float value", key);
            return false;
        }
        array.push_back(value.GetFloat());
        return true;
    };

    auto stringArrayIterator = [&success](const char* key, const ConfigValue& value,
                                          std::vector<const char*>& array)
    {
        if (!value.IsString())
        {
            success = false;
            LOG_ERROR("Array '%s' contains non-string value", key);
            return false;
        }
        array.push_back(value.GetString());
        return true;
    };

    auto iteratorCallback = [&](const char* key, const ConfigValue& value)
    {
        if (translator.mBoolValues.count(key) > 0)
        {
            if (!value.IsBool())
            {
                success = false;
                LOG_ERROR("Value '%s' is not of boolean type", key);
                return false;
            }
            auto memberPtr = translator.mBoolValues[key];
            object.*memberPtr = value.GetBool();
        }
        else if (translator.mIntValues.count(key) > 0)
        {
            if (!value.IsInt())
            {
                success = false;
                LOG_ERROR("Value '%s' is not of integer type", key);
                return false;
            }
            auto memberPtr = translator.mIntValues[key];
            object.*memberPtr = value.GetInt();
        }
        else if (translator.mFloatValues.count(key) > 0)
        {
            if (!value.IsFloat())
            {
                success = false;
                LOG_ERROR("Value '%s' is not of float type", key);
                return false;
            }
            auto memberPtr = translator.mFloatValues[key];
            object.*memberPtr = value.GetFloat();
        }
        else if (translator.mStringValues.count(key) > 0)
        {
            if (!value.IsString())
            {
                success = false;
                LOG_ERROR("Value '%s' is not of string type", key);
                return false;
            }
            auto memberPtr = translator.mStringValues[key];
            object.*memberPtr = value.GetString();
        }
        else if (translator.mIntArrayValues.count(key) > 0)
        {
            auto memberPtr = translator.mIntArrayValues[key];
            IterateArray(std::bind(intArrayIterator, key, _2, std::ref(object.*memberPtr)),
                         value.GetArray());
        }
        else if (translator.mBoolArrayValues.count(key) > 0)
        {
            auto memberPtr = translator.mBoolArrayValues[key];
            IterateArray(std::bind(boolArrayIterator, key, _2, std::ref(object.*memberPtr)),
                         value.GetArray());
        }
        else if (translator.mFloatArrayValues.count(key) > 0)
        {
            auto memberPtr = translator.mFloatArrayValues[key];
            IterateArray(std::bind(floatArrayIterator, key, _2, std::ref(object.*memberPtr)),
                         value.GetArray());
        }
        else if (translator.mStringArrayValues.count(key) > 0)
        {
            auto memberPtr = translator.mStringArrayValues[key];
            IterateArray(std::bind(stringArrayIterator, key, _2, std::ref(object.*memberPtr)),
                         value.GetArray());
        }
        return true;
    };

    Iterate(iteratorCallback, node);

    return success;
}

} // namespace Common
} // namespace NFE
