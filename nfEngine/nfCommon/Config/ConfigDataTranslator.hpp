/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once

#include "Config.hpp"
#include "../Logger/Logger.hpp"
#include "../Containers/Map.hpp"


namespace NFE {
namespace Common {

struct CStringComparator
{
    bool Equal(const char* a, const char* b) const
    {
        return strcmp(a, b) == 0;
    }

    bool Less(const char* a, const char* b) const
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
    typedef DynArray<bool> T::*BoolArrayValue;
    typedef DynArray<int> T::*IntArrayValue;
    typedef DynArray<float> T::*FloatArrayValue;
    typedef DynArray<const char*> T::*StringArrayValue;

    Map<const char*, BoolValue, CStringComparator> mBoolValues;
    Map<const char*, IntValue, CStringComparator> mIntValues;
    Map<const char*, FloatValue, CStringComparator> mFloatValues;
    Map<const char*, StringValue, CStringComparator> mStringValues;
    Map<const char*, BoolArrayValue, CStringComparator> mBoolArrayValues;
    Map<const char*, IntArrayValue, CStringComparator> mIntArrayValues;
    Map<const char*, FloatArrayValue, CStringComparator> mFloatArrayValues;
    Map<const char*, StringArrayValue, CStringComparator> mStringArrayValues;

public:
    DataTranslator& Add(const char* name, BoolValue value)
    {
        mBoolValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(const char* name, IntValue value)
    {
        mIntValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(const char* name, FloatValue value)
    {
        mFloatValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(const char* name, StringValue value)
    {
        mStringValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(const char* name, BoolArrayValue value)
    {
        mBoolArrayValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(const char* name, IntArrayValue value)
    {
        mIntArrayValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(const char* name, FloatArrayValue value)
    {
        mFloatArrayValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(const char* name, StringArrayValue value)
    {
        mStringArrayValues.Insert(name, value);
        return *this;
    }
};


template <typename T>
bool Config::TranslateConfigObject(ConfigObjectNodePtr node, DataTranslator<T>& translator, T& object) const
{
    using namespace std::placeholders;

    bool success = true;

    auto intArrayIterator = [&success](const char* key, const ConfigValue& value, DynArray<int32>& array)
    {
        if (!value.Is<int32>())
        {
            success = false;
            NFE_LOG_ERROR("Array '%s' contains non-integer value", key);
            return false;
        }
        array.PushBack(value.Get<int32>());
        return true;
    };

    auto boolArrayIterator = [&success](const char* key, const ConfigValue& value, DynArray<bool>& array)
    {
        if (!value.Is<bool>())
        {
            success = false;
            NFE_LOG_ERROR("Array '%s' contains non-boolean value", key);
            return false;
        }
        array.PushBack(value.Get<bool>());
        return true;
    };

    auto floatArrayIterator = [&success](const char* key, const ConfigValue& value, DynArray<float>& array)
    {
        if (!value.Is<float>())
        {
            success = false;
            NFE_LOG_ERROR("Array '%s' contains non-float value", key);
            return false;
        }
        array.PushBack(value.Get<float>());
        return true;
    };

    auto stringArrayIterator = [&success](const char* key, const ConfigValue& value, DynArray<const char*>& array)
    {
        if (!value.IsString())
        {
            success = false;
            NFE_LOG_ERROR("Array '%s' contains non-string value", key);
            return false;
        }
        array.PushBack(value.GetString());
        return true;
    };

    auto iteratorCallback = [&](const char* key, const ConfigValue& value)
    {
        if (translator.mBoolValues.Exists(key))
        {
            if (!value.Is<bool>())
            {
                success = false;
                NFE_LOG_ERROR("Value '%s' is not of boolean type", key);
                return false;
            }
            auto memberPtr = translator.mBoolValues[key];
            object.*memberPtr = value.Get<bool>();
        }
        else if (translator.mIntValues.Exists(key))
        {
            if (!value.Is<int32>())
            {
                success = false;
                NFE_LOG_ERROR("Value '%s' is not of integer type", key);
                return false;
            }
            auto memberPtr = translator.mIntValues[key];
            object.*memberPtr = value.Get<int32>();
        }
        else if (translator.mFloatValues.Exists(key))
        {
            if (!value.Is<float>())
            {
                success = false;
                NFE_LOG_ERROR("Value '%s' is not of float type", key);
                return false;
            }
            auto memberPtr = translator.mFloatValues[key];
            object.*memberPtr = value.Get<float>();
        }
        else if (translator.mStringValues.Exists(key))
        {
            if (!value.IsString())
            {
                success = false;
                NFE_LOG_ERROR("Value '%s' is not of string type", key);
                return false;
            }
            auto memberPtr = translator.mStringValues[key];
            object.*memberPtr = value.GetString();
        }
        else if (translator.mIntArrayValues.Exists(key))
        {
            auto memberPtr = translator.mIntArrayValues[key];
            IterateArray(std::bind(intArrayIterator, key, _2, std::ref(object.*memberPtr)),
                         value.GetArray());
        }
        else if (translator.mBoolArrayValues.Exists(key))
        {
            auto memberPtr = translator.mBoolArrayValues[key];
            IterateArray(std::bind(boolArrayIterator, key, _2, std::ref(object.*memberPtr)),
                         value.GetArray());
        }
        else if (translator.mFloatArrayValues.Exists(key))
        {
            auto memberPtr = translator.mFloatArrayValues[key];
            IterateArray(std::bind(floatArrayIterator, key, _2, std::ref(object.*memberPtr)),
                         value.GetArray());
        }
        else if (translator.mStringArrayValues.Exists(key))
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
