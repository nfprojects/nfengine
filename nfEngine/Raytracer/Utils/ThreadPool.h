#pragma once

#include "../Raytracer.h"
#include "../../nfCommon/Containers/DynArray.hpp"

#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace NFE {
namespace RT {

using ParallelTask = std::function<void(uint32 taskID, uint32 threadID)>;

class ThreadPool
{
public:
    struct TaskCoords
    {
        uint32 x;
        uint32 y;
    };

    ThreadPool();
    ~ThreadPool();

    void SetNumThreads(const uint32 numThreads);

    void RunParallelTask(const ParallelTask& task, uint32 num);

    NFE_FORCE_INLINE uint32 GetNumThreads() const
    {
        return mThreads.Size();
    }

private:

    void StartWorkerThreads(uint32 num);
    void StopWorkerThreads();

    using Lock = std::unique_lock<std::mutex>;

    Common::DynArray<std::thread> mThreads;
    std::condition_variable mNewTaskCV;
    std::condition_variable mTileFinishedCV;
    std::mutex mMutex;

    ParallelTask mTask;
    uint32 mNumTasks;
    uint32 mCurrentTask;

    std::atomic<uint32> mTasksLeft;

    bool mFinishThreads;

    void ThreadCallback(uint32 id);
};

} // namespace RT
} // namespace NFE
