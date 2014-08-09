#pragma once

#include "packerToolDefines.hpp"
#include <vector>
#include <string>

typedef void (*CommandFunc)(std::string& cmdString);

class packerToolCommand
{
public:
    packerToolCommand();
    packerToolCommand(std::string commandString, CommandFunc command);
    packerToolCommand(std::string commandString, CommandFunc command, std::string commandHelpMessage);
    ~packerToolCommand();

    std::string GetCommandString() const;
    std::string GetCommandHelpMessage() const;

    void CallCommand(std::string& cmdString) const;

    void SetCommand(CommandFunc command);
    void SetCommandString(std::string commandString);
    void SetCommandHelpMessage(std::string helpMessage);

private:
    CommandFunc m_command;

    std::string m_commandString;
    std::string m_commandHelpMessage;
};

typedef std::vector<packerToolCommand> packerToolCommandList;
typedef packerToolCommandList::iterator packerToolCommandIt;