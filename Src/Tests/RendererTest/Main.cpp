#include "PCH.hpp"
#include "Backends.hpp"

#include "Engine/Common/FileSystem/FileSystem.hpp"
#include "Engine/Common/Logger/Logger.hpp"


const NFE::Common::String BACKEND_ARG_NAME = "--renderer";
const NFE::Common::String CARD_ID_ARG_NAME = "--card_id";

int main(int argc, char* argv[])
{
    if (!NFE::Common::InitSubsystems())
    {
        NFE::Common::ShutdownSubsystems();
        return -1;
    }

    testing::InitGoogleTest(&argc, argv);

    const NFE::Common::String execPath = NFE::Common::FileSystem::GetExecutablePath();
    const NFE::Common::String execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    gPreferedCardId = -1;
    gDebugLevel = 0;
#ifdef _DEBUG
    gDebugLevel = 1;
#endif

    // TODO use some helper class instead of manual checks
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--renderer") == 0 || strcmp(argv[i], "-r") == 0)
        {
            if (i + 1 < argc)
            {
                gBackend = argv[++i];
            }
            else
            {
                NFE_LOG_ERROR("Missing command line parameter");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--card") == 0 || strcmp(argv[i], "-c") == 0)
        {
            if (i + 1 < argc)
            {
                gPreferedCardId = atoi(argv[++i]);
            }
            else
            {
                NFE_LOG_ERROR("Missing command line parameter");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0)
        {
            if (i + 1 < argc)
            {
                gDebugLevel = atoi(argv[++i]);
            }
            else
            {
                NFE_LOG_ERROR("Missing command line parameter");
                return 1;
            }
        }
        else
        {
            NFE_LOG_ERROR("Unknown command line parameter: %s", argv[i]);
            return 1;
        }
    }

    if (gBackend.Empty())
    {
        const auto& defBackend = GetDefaultBackend();
        gBackend = defBackend[0];
        gShaderPathPrefix = defBackend[1];
        gShaderPathExt = defBackend[2];
    }
    else if (D3D12_BACKEND == gBackend)
    {
        gShaderPathPrefix = HLSL5_SHADER_PATH_PREFIX;
        gShaderPathExt = HLSL5_SHADER_EXTENSION;
    }
    else if (VK_BACKEND == gBackend)
    {
        gShaderPathPrefix = GLSL_SHADER_PATH_PREFIX;
        gShaderPathExt = GLSL_SHADER_EXTENSION;
    }
    else
    {
        NFE_LOG_ERROR("Incorrect backend provided");
        return 1;
    }

    int ret = RUN_ALL_TESTS();

    NFE::Common::ShutdownSubsystems();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return ret;
}
