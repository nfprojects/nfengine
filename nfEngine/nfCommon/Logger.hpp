/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Logger class
 */

#pragma once

#include "nfCommon.hpp"
#include "Timer.hpp"
#include "LoggerBackends/BackendInterface.hpp"

namespace NFE {
namespace Common {

/**
 * Main logger class.
 */
class NFCOMMON_API Logger
{
private:
    std::mutex mMutex;
    Timer mTimer;
	std::vector<std::unique_ptr<LoggerBackend>> mBackends;

	Logger(const Logger&) = delete;
	Logger& operator= (const Logger&) = delete;

public:
    Logger();
    ~Logger();

	void RegisterBackend(std::unique_ptr<LoggerBackend> backend);

    /**
     * Log single line using formated string.
     * @param type Log level type
	 * @param function Function name
	 * @param srcFile Source file name
     * @param line Number of line in the source file
	 * @param str Formated string
     * @remarks Use logging macros to simplify code
     */
	void Log(LogType type, const char* function, const char* srcFile, int line,
			 const char* str, ...);

    /**
	 * Access logger singleton instance.
     */
    static Logger* GetInstance();
};

} // namespace Common
} // namespace NFE


/// logging macros
#define LOG_ANY(type, ...)                                                         \
do {																			   \
	Common::Logger* logger = Common::Logger::GetInstance();						   \
	if (logger) logger->Log(type, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);  \
} while (0)

#define LOG_INFO(...)    LOG_ANY(Common::LogType::Info, __VA_ARGS__)
#define LOG_SUCCESS(...) LOG_ANY(Common::LogType::Success, __VA_ARGS__)
#define LOG_WARNING(...) LOG_ANY(Common::LogType::Warning, __VA_ARGS__)
#define LOG_ERROR(...)   LOG_ANY(Common::LogType::Error, __VA_ARGS__)
#define LOG_FATAL(...)   LOG_ANY(Common::LogType::Fatal, __VA_ARGS__)
