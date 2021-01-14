#pragma once

#include "../Raytracer.h"
#include "../../Common/Containers/DynArray.hpp"


namespace NFE {
namespace RT {


template<typename ValueType>
class LookupTable
{
public:
    void Init(const Common::ArrayView<const float> keys, const Common::ArrayView<const ValueType> values)
    {
        NFE_ASSERT(keys.Size() == values.Size());
        mKeys = keys;
        mValues = values;
    }

    void Init(Common::DynArray<float>&& keys, Common::DynArray<ValueType>&& values)
    {
        NFE_ASSERT(keys.Size() == values.Size());
        mKeys = std::move(keys);
        mValues = std::move(values);
    }

    const ValueType Sample(const float x) const
    {
        if (mValues.Empty())
        {
            return ValueType();
        }

        if (mValues.Size() == 1u)
        {
            return mValues.Front();
        }

        uint32 firstIndex = UINT32_MAX;
        for (uint32 i = 1; i < mKeys.Size(); ++i)
        {
            if (mKeys[i] > x)
            {
                firstIndex = i - 1u;
                break;
            }
        }

        if (firstIndex == UINT32_MAX)
        {
            return mValues.Back();
        }

        const float x0 = mKeys[firstIndex];
        const float x1 = mKeys[firstIndex + 1];
        const float weight = (x - x0) / (x1 - x0);

        const ValueType y0 = mValues[firstIndex];
        const ValueType y1 = mValues[firstIndex + 1];

        return y0 + (y1 - y0) * weight;
    }

private:
    Common::DynArray<float> mKeys;
    Common::DynArray<ValueType> mValues;
};


} // namespace RT
} // namespace NFE
