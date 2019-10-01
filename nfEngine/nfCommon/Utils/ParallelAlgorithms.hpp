#pragma once

#include "../nfCommon.hpp"

#include <algorithm>

namespace NFE {
namespace Common {

template<typename Iter, uint32 Treshold = 2048>
void ParallelSort(const Iter first, const Iter last, TaskBuilder& builder)
{
    if (last - first < Treshold) // fallback for small arrays
    {
        std::sort(first, last);
    }
    else // parallel merge sort
    {
        const Iter middle = first + std::distance(first, last) / 2u;

        builder.Task("left", [first, middle] (const TaskContext& ctx)
        {
            TaskBuilder builder(ctx);
            ParallelSort(first, middle, builder);
        });

        builder.Task("right", [middle, last] (const TaskContext& ctx)
        {
            TaskBuilder builder(ctx);
            ParallelSort(middle, last, builder);
        });

        builder.Fence(); // sync with 'left' and 'right'

        builder.Task("merge", [first, middle, last] (const TaskContext&)
        {
            std::inplace_merge(first, middle, last);
        });
    }
}

template<typename Iter, uint32 Treshold = 2048>
void ParallelStableSort(const Iter first, const Iter last, TaskBuilder& builder)
{
    if (last - first < Treshold) // fallback for small arrays
    {
        std::stable_sort(first, last);
    }
    else // parallel merge sort
    {
        const Iter middle = first + std::distance(first, last) / 2u;

        builder.Task("left", [first, middle] (const TaskContext& ctx)
        {
            TaskBuilder builder(ctx);
            ParallelStableSort(first, middle, builder);
        });

        builder.Task("right", [middle, last] (const TaskContext& ctx)
        {
            TaskBuilder builder(ctx);
            ParallelStableSort(middle, last, builder);
        });

        builder.Fence(); // sync with 'left' and 'right'

        builder.Task("merge", [first, middle, last] (const TaskContext&)
        {
            std::inplace_merge(first, middle, last);
        });
    }
}

} // namespace Common
} // namespace NFE
