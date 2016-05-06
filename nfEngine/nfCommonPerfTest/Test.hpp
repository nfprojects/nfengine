/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Common header for all Performance tests
 */

#pragma once

#include <string>
#include <typeinfo>
#include "../nfCommon/Logger.hpp"

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

    void SetUp()
    {
        static const std::string gLogFilePath = getName() + "_"
            + ::getTime() + ".txt";
        mFile.open(gLogFilePath, std::fstream::out | std::fstream::app);
    }

    void TearDown()
    {
        if (mFile.is_open())
            mFile.close();

        NFE::Common::Logger::GetInstance()->Reset();
    }

    void TestBody()
    {
    }

    virtual std::string getName()
    {
        return std::string("CommonPerfTest");
    }

    CommonPerfTest& castBase()
    {
        return *reinterpret_cast<CommonPerfTest*>(this);
    }

    template<typename T>
    CommonPerfTest& operator<<(T obj)
    {
        std::cout << obj;
        if (mFile.is_open())
            mFile << obj;

        return castBase();
    }

    // Overload to resolve std::endl's template type
    CommonPerfTest& operator<<(std::ostream&(*pManip)(std::ostream&))
    {
        std::cout << (*pManip);
        if (mFile.is_open())
            mFile << (*pManip);

        return castBase();
    }
};
