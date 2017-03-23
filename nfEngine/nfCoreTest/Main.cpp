#include "PCH.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    String execPath = NFE::Common::FileSystem::GetExecutablePath();
    String execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");
    int result = RUN_ALL_TESTS();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return result;
}
