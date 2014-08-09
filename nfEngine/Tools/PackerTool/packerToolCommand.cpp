#include "packerToolCommand.hpp"

packerToolCommand::packerToolCommand()
    : m_command(nullptr)
{ }

packerToolCommand::packerToolCommand(std::string commandString, CommandFunc command)
    : m_commandString(std::move(commandString))
    , m_command(command)
{ }

packerToolCommand::packerToolCommand(std::string commandString, CommandFunc command,
                                     std::string commandHelpMessage)
    : m_commandString(std::move(commandString))
    , m_command(command)
    , m_commandHelpMessage(std::move(commandHelpMessage))
{ }

packerToolCommand::~packerToolCommand()
{ }

std::string packerToolCommand::GetCommandString() const
{
    return m_commandString;
}

std::string packerToolCommand::GetCommandHelpMessage() const
{
    return m_commandHelpMessage;
}

void packerToolCommand::CallCommand(std::string& cmdString) const
{
    AssertMsg(m_command != nullptr, "Cannot call null callback.");

    try
    {
        m_command(cmdString);
    }
    catch (packerToolException& e)
    {
        std::cout << "Failed to successfully finish command " << this->m_commandString <<
                  ".\nPacker exception caught:\n";
        PrintColored(e.GetMsg(), ConsoleColor::RED | ConsoleColor::INTENSE);
        std::cout << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to successfully finish command " << this->m_commandString <<
                  ".\nPacker exception caught:\n";
        PrintColored(e.what(), ConsoleColor::RED | ConsoleColor::INTENSE);
        std::cout << std::endl;
    }
    catch (...)
    {
        std::cout << "Failed to successfully finish command.\n";
        PrintColored("Unknown error.", ConsoleColor::RED | ConsoleColor::INTENSE);
        std::cout << std::endl;
    }
}

void packerToolCommand::SetCommand(CommandFunc command)
{
    AssertMsg(command != nullptr, "Cannot set command callback to NULL.");

    m_command = command;
}

void packerToolCommand::SetCommandString(std::string commandString)
{
    AssertMsg(!commandString.empty(), "Cannot set command string to empty string.");

    m_commandString = commandString;
}

void packerToolCommand::SetCommandHelpMessage(std::string commandHelpMessage)
{
    m_commandHelpMessage = commandHelpMessage;
}
