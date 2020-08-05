#pragma once

#include "LocalAllocator.hpp"
#include "CommandBatch.hpp"
#include "Translations.hpp"

#include <Engine/Common/Containers/DynArray.hpp>


namespace NFE {
namespace Renderer {

class CommandRecording
{
    LocalAllocator<NFE_VK_COMMAND_MEMORY_SPACE> mCommandAllocator;
    Common::DynArray<CommandBatch> mBatches;

public:
    CommandRecording()
        : mCommandAllocator()
        , mBatches()
    {
        mBatches.Reserve(8);
        mBatches.EmplaceBack(mCommandAllocator);
    }

    template <typename T, typename... Args>
    NFE_FORCE_INLINE void Record(Args&&... args)
    {
        mBatches.Back().Record<T>(args...);
    }

    NFE_FORCE_INLINE void Submit(VkCommandBuffer commandBuffer, CommandBatchState& state)
    {
        for (uint32 i = 0; i < mBatches.Size(); ++i)
            mBatches[i].Submit(commandBuffer, state);

        mBatches.Clear();
        mCommandAllocator.Clear();
        mBatches.EmplaceBack(mCommandAllocator);
    }

    NFE_FORCE_INLINE void FinishBatch(CommandBatchType batchType)
    {
        mBatches.Back().SetType(batchType);
        mBatches.EmplaceBack(mCommandAllocator);
    }

    NFE_FORCE_INLINE uint32 CurrentBatch() const
    {
        return mBatches.Size() - 1;
    }

    NFE_FORCE_INLINE void PrintBatches()
    {
        for (uint32 i = 0; i < mBatches.Size(); ++i)
        {
            NFE_LOG_DEBUG("Command batch #%d (%s):", i, BatchTypeToString(mBatches[i].GetType()));
            mBatches[i].Print();
        }
    }
};


} // namespace Renderer
} // namespace NFE
