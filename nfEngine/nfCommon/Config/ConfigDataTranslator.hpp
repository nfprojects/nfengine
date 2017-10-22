/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once

#include "Config.hpp"
#include "../Logger/Logger.hpp"
#include "../Containers/HashMap.hpp"


namespace NFE {
namespace Common {

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

    HashMap<StringView, BoolValue> mBoolValues;
    HashMap<StringView, IntValue> mIntValues;
    HashMap<StringView, FloatValue> mFloatValues;
    HashMap<StringView, StringValue> mStringValues;
    HashMap<StringView, BoolArrayValue> mBoolArrayValues;
    HashMap<StringView, IntArrayValue> mIntArrayValues;
    HashMap<StringView, FloatArrayValue> mFloatArrayValues;
    HashMap<StringView, StringArrayValue> mStringArrayValues;

public:
    DataTranslator& Add(StringView name, BoolValue value)
    {
        mBoolValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(StringView name, IntValue value)
    {
        mIntValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(StringView name, FloatValue value)
    {
        mFloatValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(StringView name, StringValue value)
    {
        mStringValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(StringView name, BoolArrayValue value)
    {
        mBoolArrayValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(StringView name, IntArrayValue value)
    {
        mIntArrayValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(StringView name, FloatArrayValue value)
    {
        mFloatArrayValues.Insert(name, value);
        return *this;
    }

    DataTranslator& Add(StringView name, StringArrayValue value)
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

    auto intArrayIterator = [&success](StringView key, const ConfigValue& value, DynArray<int32>& array)
    {
        if (!value.Is<int32>())
        {
            success = false;
            NFE_LOG_ERROR("Array '%.*s' contains non-integer value", key.Length(), key.Data());
            return false;
        }
        array.PushBack(value.Get<int32>());
        return true;
    };

    auto boolArrayIterator = [&success](StringView key, const ConfigValue& value, DynArray<bool>& array)
    {
        if (!value.Is<bool>())
        {
            success = false;
            NFE_LOG_ERROR("Array '%.*s' contains non-boolean value", key.Length(), key.Data());
            return false;
        }
        array.PushBack(value.Get<bool>());
        return true;
    };

    auto floatArrayIterator = [&success](StringView key, const ConfigValue& value, DynArray<float>& array)
    {
        if (!value.Is<float>())
        {
            success = false;
            NFE_LOG_ERROR("Array '%.*s' contains non-float value", key.Length(), key.Data());
            return false;
        }
        array.PushBack(value.Get<float>());
        return true;
    };

    auto stringArrayIterator = [&success](StringView key, const ConfigValue& value, DynArray<const char*>& array)
    {
        if (!value.IsString())
        {
            success = false;
            NFE_LOG_ERROR("Array '%.*s' contains non-string value", key.Length(), key.Data());
            return false;
        }
        array.PushBack(value.GetString());
        return true;
    };

    auto iteratorCallback = [&](StringView key, const ConfigValue& value)
    {
        if (translator.mBoolValues.Exists(key))
        {
            if (!value.Is<bool>())
            {
                success = false;
                NFE_LOG_ERROR("Value '%.*s' is not of boolean type", key.Length(), key.Data());
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
                NFE_LOG_ERROR("Value '%.*s' is not of integer type", key.Length(), key.Data());
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
                NFE_LOG_ERROR("Value '%.*s' is not of float type", key.Length(), key.Data());
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
                NFE_LOG_ERROR("Value '%.*s' is not of string type", key.Length(), key.Data());
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
