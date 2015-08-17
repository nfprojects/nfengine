#include "PCH.hpp"
#include "packerToolUI.hpp"

#include <streambuf>
#include <iterator>
#include <sstream>

packerToolUI::packerToolUI()
    : mIsUsable(true)
    , mBeforeLoopCommand(nullptr)
    , mAfterLoopCommand(nullptr)
    , mConfigFile(nullptr)
{
    //TODO Add config support. Uncomment below lines and change isUsable default value when config support is finished.
    /*
    std::cout << "Loading configuration... ";
    m_configFile = fopen(s_configFilePath.c_str(), "rb");

    try
    {
        if(!m_configFile)
        {
            std::string input;

            std::cout << "FAILED.\n\nIt seems, that you miss a config.\nSince this may be your first run, spare a moment to config this app.\n\n";
            std::cout << "First of all, what will be your output file?\n";
            std::getline(std::cin, input);

            m_curConfig.s_OutFilePath = input;
            std::cout << "\nOK, enough for now. Creating config... ";

            CreateConfig();
            std::cout << "done. Enjoy!\n\n";
            m_isUsable = true;
            SAFE_FILEFREE(m_configFile);
        }
        else
        {
            // TODO load config and set up UI
            ReadConfig();
            m_isUsable = true;
            std::cout << "done.\n\n";
            SAFE_FILEFREE(m_configFile);
        }
    }
    catch(packerException& e)
    {
        PrintColored("FAILED. ", ConsoleColor::RED | ConsoleColor::INTENSE);
        std::cout << "Exception caught: " << e.GetMsg() << std::endl;
        SAFE_FILEFREE(m_configFile);
    }
    */
}

packerToolUI::~packerToolUI()
{ }

void packerToolUI::CreateConfig()
{
    /// TODO Add config support
    //Throw("Creating config failed.");
}

void packerToolUI::ReadConfig()
{
    /// TODO Add config support
    //Throw("Reading config failed.");
}

void packerToolUI::AddCommand(const std::string& commandString, const CommandFunc& command)
{
    AssertMsg(mIsUsable, "Cannot add command - UI is not yet usable.");
    AssertMsg(commandString != m_exitCommandStr,
              "Cannot add command - command string used as exit command.");
    AssertMsg(commandString != m_helpCommandStr,
              "Cannot add command - command string used as help command.");

    auto it = std::find_if(mCommandList.begin(), mCommandList.end(),
                           [&commandString] (packerToolCommand & cmd) -> bool { return commandString == cmd.GetCommandString(); } );

    if (it == mCommandList.end())
        mCommandList.push_back(packerToolCommand(commandString, command));
    else
        ThrowMsg("Cannot add command - command string already used.");
}

void packerToolUI::AddCommand(const std::string& commandString, const CommandFunc& command,
                              const std::string& commandHelpMessage)
{
    AssertMsg(mIsUsable, "Cannot add command - UI is not yet usable.");
    AssertMsg(commandString != m_exitCommandStr,
              "Cannot add command - command string used as exit command.");
    AssertMsg(commandString != m_helpCommandStr,
              "Cannot add command - command string used as help command.");

    auto it = std::find_if(mCommandList.begin(), mCommandList.end(),
                           [&commandString] (packerToolCommand & cmd) -> bool { return commandString == cmd.GetCommandString(); } );

    if (it == mCommandList.end())
        mCommandList.push_back(packerToolCommand(commandString, command, commandHelpMessage));
    else
        ThrowMsg("Cannot add command - command string already used.");
}

void packerToolUI::AddBeforeLoopCommand(const UIFunc& command)
{
    mBeforeLoopCommand = command;
}

void packerToolUI::AddAfterLoopCommand(const UIFunc& command)
{
    mAfterLoopCommand = command;
}

void packerToolUI::PrintCommands()
{
    std::cout << "All available commands:\n";
    std::cout << "    * help - what you see now." << std::endl;

    //go through all commands for command strings and help messages
    std::for_each(mCommandList.begin(), mCommandList.end(),
                  [](packerToolCommand & cmd) -> void
    {
        if (cmd.GetCommandHelpMessage().empty())
            std::cout << "    * " << cmd.GetCommandString() << std::endl;
        else
            std::cout << "    * " << cmd.GetCommandString() << " - " << cmd.GetCommandHelpMessage() << std::endl;
    } );

    std::cout << "    * exit - quits the app." << std::endl;
}

void packerToolUI::MainLoop()
{
    AssertMsg(mIsUsable, "Cannot enter UI main loop - UI is not initialized.");

    if (mBeforeLoopCommand)
        mBeforeLoopCommand();

    bool loopActive = true;
    std::string input, input_first, input_rest;
    packerToolCommandIt curCommandIt;

    do
    {
        //receive message from user
        std::cout << "\n> ";
        std::getline(std::cin, input);

        //gather first word
        input_first.clear();
        std::string::iterator it = input.begin();
        while (it != input.end() && *it != ' ' && *it != '\n')
            input_first += *it++;

#ifdef _DEBUG
        std::cout << "Received ";
        PrintColored(NFE::Common::ConsoleColor::Green, input_first.c_str());
        std::cout << " command.\n";
#endif

        //if user calls exit, quit the app
        if (input_first == m_exitCommandStr)
        {
            std::cout << "Exiting. Bye!";
            loopActive = false;
            continue;
        }

        //print help message
        if (input_first == m_helpCommandStr)
        {
            PrintCommands();
            continue;
        }

        //default commands are not called - try to find current command in list
        curCommandIt = std::find_if(mCommandList.begin(), mCommandList.end(),
                                    [&input_first](packerToolCommand & curCommand) -> bool { return (input_first == curCommand.GetCommandString()); });

        if (curCommandIt == mCommandList.end())
        {
            std::cout << "Unknown command.\n";
            continue;
        }

        //skip all spaces from input until next word appears (if any exists)
        while (it != input.end() && *it == ' ')
            it++;

        //flush remaining cin data to new string until we get to new line
        input_rest.clear();
        while (it != input.end() && *it != '\n')
            input_rest += *it++;

#ifdef _DEBUG
        std::cout << "Found ";
        PrintColored(NFE::Common::ConsoleColor::Green, curCommandIt->GetCommandString().c_str());
        std::cout << " command in database. Calling.\n";
#endif

        curCommandIt->CallCommand(input_rest);
    }
    while (loopActive);

    if (mAfterLoopCommand)
        mAfterLoopCommand();
}
