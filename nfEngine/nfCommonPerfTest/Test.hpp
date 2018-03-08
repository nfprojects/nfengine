/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common header for all Performance tests
 */

#pragma once

#include <string>
#include "nfCommon/Logger/LoggerImpl.hpp"

#include <fstream>


static const char* LOGS_DIR = "./Logs/PerfTests/";

namespace
{
std::string getTime()
{
    time_t rawTime;
    time(&rawTime);
    struct tm* timeInfo = localtime(&rawTime);
    if (timeInfo != nullptr)
    {
        /// print current date and time, in format
        const int MAX_DATE_LENGTH = 32;
        char dateTimeStr[MAX_DATE_LENGTH];
        strftime(dateTimeStr, MAX_DATE_LENGTH, "%m-%d-%Y_%H.%M.%S", timeInfo);
        return std::string(dateTimeStr);
    }
    return "";
}
} // namespace

class CommonPerfTest : public testing::Test
{
public:
    std::fstream mFile;

    void SetUp() override
    {
        static const std::string gLogFilePath = LOGS_DIR + GetName() + "_" + ::getTime() + ".txt";
        mFile.open(gLogFilePath, std::fstream::out | std::fstream::app);
    }

    void TearDown() override
    {
        if (mFile.is_open())
            mFile.close();

        NFE::Common::Logger::GetInstance()->Reset();
    }

    virtual const std::string GetName() const
    {
        return std::string("CommonPerfTest");
    }

    CommonPerfTest& GetOStream()
    {
        return *reinterpret_cast<CommonPerfTest*>(this);
    }

    template<typename T>
    CommonPerfTest& operator<<(T obj)
    {
        std::cout << obj;
        if (mFile.is_open())
            mFile << obj;

        return GetOStream();
    }

    // Overload to resolve std::endl's template type
    CommonPerfTest& operator<<(std::ostream&(*manip)(std::ostream&))
    {
        std::cout << (*manip);
        if (mFile.is_open())
            mFile << (*manip);

        return GetOStream();
    }
};
