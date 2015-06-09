#include "PCH.hpp"
#include "../nfCommon/FileSystem.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    NFE::Common::FileSystem::ChangeDirectory("./../../..");
    int result = RUN_ALL_TESTS();

    _CrtDumpMemoryLeaks();
    return result;
}
