#include "packerTool.hpp"

#include <memory>

#include "packerToolCallbacks.hpp"

int main() try
{
    std::cout << "PackerTool\nCreated by LKostyra, 2013-2014.\nFor additional information type \"help\" command.\n\n";

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
    std::cout << "Packer exception caught at main: ";
    PrintColored(e.GetMsg(), ConsoleColor::RED | ConsoleColor::INTENSE);
    std::cout << "Aborting program execution." << std::endl;
    getchar();
}
catch (std::exception& e)
{
    std::cout << "STL exception caught at main: ";
    PrintColored(e.what(), ConsoleColor::RED | ConsoleColor::INTENSE);
    std::cout << "Aborting program execution." << std::endl;
    getchar();
}
catch (...)
{
    std::cout << "Unknown exception caught at main.\nAborting program execution." << std::endl;
    getchar();
}
