#pragma once

#include <string>
#include <string.h>
#include <cstring>

class packerToolException
{
public:
    packerToolException(unsigned int line, const char* file);
    packerToolException(unsigned int line, const char* file, const char* msg);
    std::string GetMsg();

private:
    std::string m_exceptionInfo;
};

//Throws simple packerException object containing current line and file.
#define Throw() throw packerToolException(__LINE__, __FILE__)

//Used to check for errors. If cond is true, exception is thrown.
#define Assert(cond) \
    if(!(cond)) \
    { \
        std::string msg_str; \
        msg_str = std::string("Assertion ") + #cond + " failed."; \
        throw packerToolException(__LINE__, __FILE__, msg_str.c_str()); \
    }

//Throws packerException object with user-defined message.
#define ThrowMsg(msg) throw packerToolException(__LINE__, __FILE__, msg)

//Used to check for errors. If cond is true, packerException object with user-defined message is thrown.
#define AssertMsg(cond, msg) \
    if(!(cond)) \
    { \
        std::string msg_str; \
        msg_str = std::string("Assertion ") + #cond + " failed: " + msg; \
        throw packerToolException(__LINE__, __FILE__, msg_str.c_str()); \
    }