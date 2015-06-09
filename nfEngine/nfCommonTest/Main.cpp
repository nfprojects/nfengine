#include "PCH.hpp"
#include "../nfCommon/FileSystem.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    NFE::Common::FileSystem::ChangeDirectory(execPath + "/../../..");
    int result = RUN_ALL_TESTS();

#if defined(WIN32) && defined(_CRTDBG_MAP_ALLOC)
    _CrtDumpMemoryLeaks();
#endif // defined(WIN32) && defined(_CRTDBG_MAP_ALLOC)
    return result;
}
