#include "packerToolException.hpp"

packerToolException::packerToolException(unsigned int line, const char* file)
{
    m_exceptionInfo.clear();

    m_exceptionInfo = std::string("Exception thrown at ") + file + ":" + std::to_string(line) + ".\n";
}

packerToolException::packerToolException(unsigned int line, const char* file, const char* msg)
{
    m_exceptionInfo.clear();

    m_exceptionInfo = std::string("Exception thrown at ") + file + ":" + std::to_string(
                          line) + ". " + msg + "\n";
}

std::string packerToolException::GetMsg()
{
    return m_exceptionInfo;
}