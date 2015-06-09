#include "PCH.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();

#if defined(WIN32) && defined(_CRTDBG_MAP_ALLOC)
    _CrtDumpMemoryLeaks();
#endif // defined(WIN32) && defined(_CRTDBG_MAP_ALLOC)
    return result;
}
