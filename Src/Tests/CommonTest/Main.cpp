#include "PCH.hpp"
#include "Engine/Common/FileSystem/FileSystem.hpp"
#include "Engine/Common/Containers/String.hpp"
#include "Engine/Common/Math/Math.hpp"


int main(int argc, char* argv[])
{
    if (!NFE::Common::InitSubsystems())
    {
        NFE::Common::ShutdownSubsystems();
        return -1;
    }

    testing::InitGoogleTest(&argc, argv);

    const NFE::Common::String execPath = NFE::Common::FileSystem::GetExecutablePath();
    const NFE::Common::StringView execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    NFE::Math::SetFlushDenormalsToZero();

    int result = RUN_ALL_TESTS();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    NFE::Common::ShutdownSubsystems();

    NFE_ASSERT(NFE::Math::GetFlushDenormalsToZero(), "Something disabled flushing denormal float to zero");

    return result;
}
