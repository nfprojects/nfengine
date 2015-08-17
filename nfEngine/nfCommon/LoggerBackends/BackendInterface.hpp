/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of logger backend interface
 */

#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

enum class LogType
{
	Info,
	Success,
	Warning,
	Error,
	Fatal,
};

/**
 * Logger backend interface.
 */
class NFCOMMON_API LoggerBackend
{
public:
	virtual ~LoggerBackend() {}
	virtual void Log(LogType type, const char* function, const char* srcFile, int line,
					 const char* str, double timeElapsed) = 0;
};

} // namespace Common
} // namespace NFE
