#include "PCH.hpp"
#include "packerTool.hpp"

#include <memory>

#include "packerToolCallbacks.hpp"

int main() try
{
    std::cout <<
              "PackerTool\nCreated by LKostyra, 2013-2014.\nFor additional information type \"help\" command.\n\n";

    std::unique_ptr<packerToolUI> p_UI(new packerToolUI());

    if (p_UI.get())
    {
        AddCommands(p_UI.get());

        p_UI->MainLoop();
    }

    p_UI.release();

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
catch (packerToolException& e)
{
    using namespace NFE::Common;

    std::cout << "Packer exception caught at main: ";
    PrintColored(ConsoleColor::Red | NFE::Common::ConsoleColor::Intense, e.GetMsg().c_str());
    std::cout << "Aborting program execution." << std::endl;
    getchar();
}
catch (std::exception& e)
{
    using namespace NFE::Common;

    std::cout << "STL exception caught at main: ";
    PrintColored(ConsoleColor::Red | ConsoleColor::Intense, e.what());
    std::cout << "Aborting program execution." << std::endl;
    getchar();
}
catch (...)
{
    std::cout << "Unknown exception caught at main.\nAborting program execution." << std::endl;
    getchar();
}
