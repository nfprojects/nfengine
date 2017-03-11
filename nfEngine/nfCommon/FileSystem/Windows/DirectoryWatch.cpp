/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of DirectoryWatch class
 */

#include "PCH.hpp"
#include "../DirectoryWatch.hpp"
#include "Logger/Logger.hpp"
#include "System/Win/Common.hpp"

#include <assert.h>


namespace NFE {
namespace Common {

#define BUFFER_SIZE 16384

WatchRequest::WatchRequest()
    : dirHandle(INVALID_HANDLE_VALUE)
{
    frontBuffer.resize(BUFFER_SIZE);
    backBuffer.resize(BUFFER_SIZE);

    ::ZeroMemory(&overlapped, sizeof(OVERLAPPED));
    overlapped.hEvent = static_cast<HANDLE>(this);
}

bool WatchRequest::Start()
{
    DWORD bytes = 0;
    BOOL success = ::ReadDirectoryChangesW(dirHandle,
                                           frontBuffer.data(), BUFFER_SIZE,
                                           FALSE,
                                           FILE_NOTIFY_CHANGE_LAST_WRITE |
                                           FILE_NOTIFY_CHANGE_CREATION |
                                           FILE_NOTIFY_CHANGE_FILE_NAME |
                                           FILE_NOTIFY_CHANGE_DIR_NAME |
                                           FILE_NOTIFY_CHANGE_ATTRIBUTES |
                                           FILE_NOTIFY_CHANGE_SIZE,
                                           &bytes, &overlapped,
                                           &DirectoryWatch::NotificationCompletion);

    if (::SetEvent(watch->mEvent) == 0)
        LOG_ERROR("SetEvent() failed: %s", GetLastErrorString().c_str());

    if (success == 0)
    {
        LOG_ERROR("ReadDirectoryChangesW() failed: %s", GetLastErrorString().c_str());
        return false;
    }

    return true;
}

void WatchRequest::Stop()
{
    if (dirHandle != INVALID_HANDLE_VALUE)
    {
        ::CancelIo(dirHandle);
        ::CloseHandle(dirHandle);
        dirHandle = INVALID_HANDLE_VALUE;
    }
}

// ================================================================================================

DirectoryWatch::DirectoryWatch()
    : mRequestsNum(0)
    , mRunning(true)
{
    mEvent = ::CreateEvent(NULL, FALSE, FALSE, L"DirectoryWatch Event");
    if (mEvent == NULL)
    {
        LOG_ERROR("CreateEvent() failed: %s", GetLastErrorString().c_str());
        return;
    }

    mThread = ::CreateThread(0, 0, Dispatcher, this, 0, 0);
    if (mThread == NULL)
        LOG_ERROR("CreateThread() failed: %s", GetLastErrorString().c_str());
}

DirectoryWatch::~DirectoryWatch()
{
    mRunning.store(false);

    DWORD ret = ::QueueUserAPC(&TerminateProc, mThread, reinterpret_cast<ULONG_PTR>(this));
    if (ret == 0)
        LOG_ERROR("QueueUserAPC() failed: %s", GetLastErrorString().c_str());

    ::WaitForSingleObject(mThread, INFINITE);
}

void DirectoryWatch::SetCallback(WatchCallback callback)
{
    mCallback = callback;
}

bool DirectoryWatch::WatchPath(const std::string& path, Event eventFilter)
{
    {
        std::unique_lock<std::mutex> lock(mMutex);
        auto it = mRequests.find(path);
        if (it != mRequests.end())
        {
            // remove watch request if filter changes
            if (it->second->filter != static_cast<int>(eventFilter))
            {
                ::ResetEvent(mEvent);
                DWORD ret = ::QueueUserAPC(&RemoveDirectoryProc, mThread,
                                           reinterpret_cast<ULONG_PTR>(it->second.get()));
                if (ret == 0)
                {
                    LOG_ERROR("QueueUserAPC() failed for path '%s': %s", path.c_str(),
                              GetLastErrorString().c_str());
                    return false;
                }

                // wait for ERROR_OPERATION_ABORTED in DirectoryWatch::NotificationCompletion()
                ::WaitForSingleObject(mEvent, INFINITE);
            }
            else // watch request not modified
                return true;
        }
    }

    if (eventFilter == Event::None)
        return true;

    std::unique_ptr<WatchRequest> request(new WatchRequest);
    UTF8ToUTF16(path, request->widePath);
    request->path = path;
    request->watch = this;
    request->filter = static_cast<int>(eventFilter);

    // open directory
    request->dirHandle = ::CreateFile(
        request->widePath.c_str(),                              // pointer to the file name
        FILE_LIST_DIRECTORY,                                    // access (read/write) mode
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
        NULL,                                                   // security descriptor
        OPEN_EXISTING,                                          // how to create
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,      // file attributes
        NULL);                                                  // file with attributes to copy
    if (request->dirHandle == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("CreateFile() failed for path '%s': %s", path.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }

    /// register watch request
    WatchRequest* requestPtr = request.get();
    mRequestsNum++;
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mRequests.insert(std::make_pair(path, std::move(request)));
    }

    /// trigger monitoring thread
    ::ResetEvent(mEvent);
    DWORD ret = ::QueueUserAPC(&AddDirectoryProc, mThread,
                               reinterpret_cast<ULONG_PTR>(requestPtr));
    if (ret == 0)
    {
        LOG_ERROR("QueueUserAPC() failed for path '%s': %s", path.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }
    // wait for signal from WatchRequest::Start()
    ::WaitForSingleObject(mEvent, INFINITE);

    LOG_DEBUG("Directory watch for path '%s' added", path.c_str());
    return true;
}

void DirectoryWatch::NotificationCompletion(DWORD errorCode, DWORD bytesTransferred,
                                            LPOVERLAPPED overlapped)
{
    WatchRequest* request = static_cast<WatchRequest*>(overlapped->hEvent);
    DirectoryWatch* watch = request->watch;

    if (errorCode == ERROR_OPERATION_ABORTED)
    {
        if (::SetEvent(watch->mEvent) == 0)
            LOG_ERROR("SetEvent() failed: %s", GetLastErrorString().c_str());

        std::unique_lock<std::mutex> lock(watch->mMutex);
        watch->mRequests.erase(request->path);
        watch->mRequestsNum--;
        return;
    }

    if (bytesTransferred == 0)
        return;

    // TODO: pointer swapping (double buffering)
    assert(bytesTransferred <= request->frontBuffer.size());
    memcpy(request->backBuffer.data(), request->frontBuffer.data(), bytesTransferred);

    // start receiving new events as fast as possible
    request->Start();

    std::string shortPath;
    char* base = request->backBuffer.data();
    for (;;)
    {
        const FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(base);
        DirectoryWatch::Event eventType;
        DirectoryWatch::Event eventFiler = static_cast<DirectoryWatch::Event>(request->filter);

        switch (fni->Action)
        {
        case FILE_ACTION_ADDED:
            eventType = Event::Create;
            break;
        case FILE_ACTION_REMOVED:
            eventType = Event::Delete;
            break;
        case FILE_ACTION_MODIFIED:
            eventType = Event::Modify;
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
            eventType = Event::MoveFrom;
            break;
        case FILE_ACTION_RENAMED_NEW_NAME:
            eventType = Event::MoveTo;
            break;
        default:
            eventType = Event::None;
        };

        if ((eventFiler & eventType) == eventType)
        {
            std::wstring fileName(fni->FileName, fni->FileNameLength / sizeof(wchar_t));
            fileName = request->widePath + L'/' + fileName;
            UTF16ToUTF8(fileName, shortPath);

            DirectoryWatch::EventData data;
            data.type = eventType;
            data.path = shortPath.c_str();
            LOG_DEBUG("DirectoryWatch: action=%d, path=%s", fni->Action, shortPath.c_str());

            if (request->watch->mCallback)
                request->watch->mCallback(data);
        }

        if (!fni->NextEntryOffset)
            break;

        base += fni->NextEntryOffset;
    }
}

// start directory watching
void DirectoryWatch::AddDirectoryProc(ULONG_PTR arg)
{
    WatchRequest* request = reinterpret_cast<WatchRequest*>(arg);
    request->Start();
}

// stop directory watching
void DirectoryWatch::RemoveDirectoryProc(ULONG_PTR arg)
{
    WatchRequest* request = reinterpret_cast<WatchRequest*>(arg);
    request->Stop();
}

// stop
void DirectoryWatch::TerminateProc(ULONG_PTR arg)
{
    DirectoryWatch* watch = reinterpret_cast<DirectoryWatch*>(arg);

    std::unique_lock<std::mutex> lock(watch->mMutex);
    for (auto& request : watch->mRequests)
        request.second->Stop();
}

DWORD DirectoryWatch::Dispatcher(LPVOID param)
{
    DirectoryWatch* watch = static_cast<DirectoryWatch*>(param);
    while (watch->mRunning || (watch->mRequestsNum > 0))
        ::SleepEx(INFINITE, true);

    return 0;
}

} // namespace Common
} // namespace NFE
