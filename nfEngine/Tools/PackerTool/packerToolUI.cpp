#include "packerToolUI.hpp"

#include <streambuf>
#include <iterator>
#include <sstream>

packerToolUI::packerToolUI()
    : m_isUsable(true)
    , m_beforeLoopCommand(nullptr)
    , m_afterLoopCommand(nullptr)
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

void packerToolUI::AddCommand(std::string commandString, CommandFunc command)
{
    AssertMsg(m_isUsable, "Cannot add command - UI is not yet usable.");
    AssertMsg(commandString != m_exitCommandStr,
              "Cannot add command - command string used as exit command.");
    AssertMsg(commandString != m_helpCommandStr,
              "Cannot add command - command string used as help command.");

    auto it = std::find_if(m_commandList.begin(), m_commandList.end(),
                           [&commandString] (packerToolCommand & cmd) -> bool { return commandString == cmd.GetCommandString(); } );

    if (it == m_commandList.end())
        m_commandList.push_back(packerToolCommand(commandString, command));
    else
        ThrowMsg("Cannot add command - command string already used.");
}

void packerToolUI::AddCommand(std::string commandString, CommandFunc command,
                              std::string commandHelpMessage)
{
    AssertMsg(m_isUsable, "Cannot add command - UI is not yet usable.");
    AssertMsg(commandString != m_exitCommandStr,
              "Cannot add command - command string used as exit command.");
    AssertMsg(commandString != m_helpCommandStr,
              "Cannot add command - command string used as help command.");

    auto it = std::find_if(m_commandList.begin(), m_commandList.end(),
                           [&commandString] (packerToolCommand & cmd) -> bool { return commandString == cmd.GetCommandString(); } );

    if (it == m_commandList.end())
        m_commandList.push_back(packerToolCommand(commandString, command, commandHelpMessage));
    else
        ThrowMsg("Cannot add command - command string already used.");
}

void packerToolUI::AddBeforeLoopCommand(UIFunc command)
{
    m_beforeLoopCommand = command;
}

void packerToolUI::AddAfterLoopCommand(UIFunc command)
{
    m_afterLoopCommand = command;
}

void packerToolUI::PrintCommands()
{
    std::cout << "All available commands:\n";
    std::cout << "    * help - what you see now." << std::endl;

    //go through all commands for command strings and help messages
    std::for_each(m_commandList.begin(), m_commandList.end(),
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
    AssertMsg(m_isUsable, "Cannot enter UI main loop - UI is not initialized.");

    if (m_beforeLoopCommand)
        m_beforeLoopCommand();

    bool b_LoopActive = true;
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
        PrintColored(input_first, ConsoleColor::GREEN);
        std::cout << " command.\n";
#endif

        //if user calls exit, quit the app
        if (input_first == m_exitCommandStr)
        {
            std::cout << "Exiting. Bye!";
            b_LoopActive = false;
            continue;
        }

        //print help message
        if (input_first == m_helpCommandStr)
        {
            PrintCommands();
            continue;
        }

        //default commands are not called - try to find current command in list
        curCommandIt = std::find_if(m_commandList.begin(), m_commandList.end(),
                                    [&input_first](packerToolCommand & curCommand) -> bool { return (input_first == curCommand.GetCommandString()); });

        if (curCommandIt == m_commandList.end())
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
        PrintColored(curCommandIt->GetCommandString(), ConsoleColor::GREEN);
        std::cout << " command in database. Calling.\n";
#endif

        curCommandIt->CallCommand(input_rest);
    }
    while (b_LoopActive);

    if (m_afterLoopCommand)
        m_afterLoopCommand();
}