#include "PCH.h"
#include "Demo.h"

#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/FileSystem/FileSystem.hpp"

#include <cxxopts.hpp>


namespace NFE {

// TODO custom library for parsing command line options

bool ParseOptions(int argc, char** argv, Options& outOptions)
{
    cxxopts::Options options("Raytracer Demo", "CPU raytracer by Michal Witanowski");
    options.add_options()
        ("w,width", "Window width", cxxopts::value<uint32>())
        ("h,height", "Window width", cxxopts::value<uint32>())
        ("s,scene", "Initial scene", cxxopts::value<std::string>())
        ("renderer", "Renderer name", cxxopts::value<std::string>())
        ("p,packet-tracing", "Use ray packet tracing by default", cxxopts::value<bool>())
        ("data", "Data path", cxxopts::value<std::string>())
        ;

    try
    {
        auto result = options.parse(argc, argv);

        if (result.count("w"))
            outOptions.windowWidth = result["w"].as<uint32>();

        if (result.count("h"))
            outOptions.windowHeight = result["h"].as<uint32>();

        if (result.count("data"))
            outOptions.dataPath = result["data"].as<std::string>().c_str();

        if (result.count("scene"))
            outOptions.sceneName = result["scene"].as<std::string>().c_str();

        if (result.count("renderer"))
            outOptions.rendererName = result["renderer"].as<std::string>().c_str();

        outOptions.enablePacketTracing = result["p"].count() > 0;
    }
    catch (cxxopts::OptionParseException& e)
    {
        NFE_LOG_ERROR("Failed to parse commandline: %hs", e.what());
        return false;
    }

    return true;
}

Options gOptions;

} // namespace NFE

using namespace NFE;

int main(int argc, char* argv[])
{
    if (!NFE::Common::InitSubsystems())
    {
        NFE::Common::ShutdownSubsystems();
        return -1;
    }

    const NFE::Common::String execPath = NFE::Common::FileSystem::GetExecutablePath();
    const NFE::Common::String execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    Math::SetFlushDenormalsToZero();

    if (!ParseOptions(argc, argv, gOptions))
    {
        return 1;
    }

    {
        DemoWindow demo;

        if (!demo.Initialize())
        {
            return 2;
        }

        NFE_LOG_INFO("Initialized.");

        if (!demo.Loop())
        {
            return 3;
        }
    }

    NFE_LOG_INFO("Closing.");

    NFE::Common::ShutdownSubsystems();

    NFE_ASSERT(Math::GetFlushDenormalsToZero(), "Something disabled flushing denormal float to zero");

#if defined(_DEBUG) && defined(WIN32)
    _CrtDumpMemoryLeaks();
#endif // _DEBUG

    return 0;
}
