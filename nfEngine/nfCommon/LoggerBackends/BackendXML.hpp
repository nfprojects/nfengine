/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Declaration of XML logger backend
 */

#pragma once

#include "../Logger.hpp"
#include "../File.hpp"


namespace NFE {
namespace Common {

/**
 * XML logger backend implementation.
 */
class NFCOMMON_API LoggerBackendXML : public LoggerBackend
{
    File mFile;
    std::vector<char> mBuffer;

public:
    LoggerBackendXML();
    ~LoggerBackendXML();
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed);
};

} // namespace Common
} // namespace NFE
