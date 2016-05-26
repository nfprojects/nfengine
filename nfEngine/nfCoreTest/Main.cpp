#include "PCH.hpp"
#include "../nfCommon/FileSystem.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");
    int result = RUN_ALL_TESTS();

    // detect memory leaks
#if defined(WIN32) && defined(_CRTDBG_MAP_ALLOC)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // defined(WIN32) && defined(_CRTDBG_MAP_ALLOC)
    return result;
}
