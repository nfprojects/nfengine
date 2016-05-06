/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Performance tests for Logger class.
 */

#include "PCH.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/Timer.hpp"
#include <fstream>

#define WRITE(...) do {        \
    std::cout << __VA_ARGS__;  \
    if (mFile.is_open())       \
        mFile << __VA_ARGS__;  \
} while (0)

using namespace NFE::Common;

namespace {
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

const int LOG_TEST_NUMBER = 10000;

class LoggerPerf : public testing::Test
{
public:
    std::fstream mFile;

    void SetUp()
    {
        static const std::string gLogFilePath = "LoggerPerfTest_"
                                                + ::getTime() + ".txt";
        mFile.open(gLogFilePath, std::fstream::out | std::fstream::app);
    }

    void TearDown()
    {
        if (mFile.is_open())
            mFile.close();

        Logger::GetInstance()->Reset();
    }
};

TEST_F(LoggerPerf, SimpleChars)
{
    WRITE("SimpleChars:" << std::endl);
    Timer timer;
    const char* simple = "c";
    double logTime, logStreamTime;

    timer.Start();
    for (int i = LOG_TEST_NUMBER; i > 0; i--)
        LOG_WARNING(simple);
    logTime = timer.Stop();

    timer.Start();
    for (int i = LOG_TEST_NUMBER; i > 0; i--)
        LOG_WARNING_S(simple);
    logStreamTime = timer.Stop();


    // print stats
    WRITE(std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
        << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl);
}

TEST_F(LoggerPerf, SymbolsShort)
{
    WRITE(std::endl << "SymbolsShort:" << std::endl);
    Timer timer;
    const std::string text = "<>'\"&";
    double logTime[5], logStreamTime[5];
    unsigned char counter = 0;

    for (auto& i : text)
    {
        timer.Start();
        for (int j = LOG_TEST_NUMBER; j > 0; j--)
            LOG_WARNING(std::to_string(i).c_str());
        logTime[counter] = timer.Stop();

        timer.Start();
        for (int j = LOG_TEST_NUMBER; j > 0; j--)
            LOG_WARNING_S(std::to_string(i).c_str());
        logStreamTime[counter++] = timer.Stop();
    }

    WRITE("Symbol | LOG_* time | LOG_*_S time" << std::endl);
    for (int i = 0; i < counter; i++)
    {
        // print stats
        WRITE(std::setprecision(5) << std::left
            << std::setw(6) << text[i] << " | "
            << std::setw(8) << logTime[i] << " s | "
            << std::setw(10) << logStreamTime[i] << " s" << std::endl);
    }
}

TEST_F(LoggerPerf, SymbolsLong)
{
    WRITE(std::endl << "SymbolsLong:" << std::endl);
    Timer timer;
    const char* text = "~!@#$%^&*()_+=-<?.';][\\,>/:\"{}|";
    double logTime, logStreamTime;

    timer.Start();
    for (int i = LOG_TEST_NUMBER; i > 0; i--)
        LOG_WARNING(text);
    logTime = timer.Stop();

    timer.Start();
    for (int i = LOG_TEST_NUMBER; i > 0; i--)
        LOG_WARNING_S(text);
    logStreamTime = timer.Stop();

// print stats
    WRITE(std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
        << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl);
}

TEST_F(LoggerPerf, LongText)
{
    WRITE(std::endl << "LongText:" << std::endl);
    Timer timer;
    const unsigned int longMessageLength = 100000;
    std::string text(longMessageLength, 'x');
    double logTime, logStreamTime;

    timer.Start();
    LOG_WARNING(text.c_str());
    logTime = timer.Stop();

    timer.Start();
    LOG_WARNING_S(text.c_str());
    logStreamTime = timer.Stop();

    // print stats
    WRITE(std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
        << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl);
}

TEST_F(LoggerPerf, FormattedMsg)
{
    WRITE(std::endl << "FormattedMsg:" << std::endl);
    Timer timer;
    int numInt = 5;
    double logTime, numDbl = 5.55;
    const char* text = "Lorem ipsum";
    const char* format = "%s%i%f%s%i%f%s%i%f";

    timer.Start();
    for (int i = LOG_TEST_NUMBER; i > 0; i--)
        LOG_INFO(format, text, numInt, numDbl,
                 text, numInt, numDbl,
                 text, numInt, numDbl);
    logTime = timer.Stop();

     // print stats
    WRITE(std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl);
}

TEST_F(LoggerPerf, StreamMsg)
{
    WRITE(std::endl << "StreamMsg:" << std::endl);
    Timer timer;
    int numInt = 5;
    double logTime, numDbl = 5.55;
    const char* text = "Lorem ipsum";

    timer.Start();
    for (int i = LOG_TEST_NUMBER; i > 0; i--)
        LOG_INFO_S(text << numInt << numDbl <<
                   text << numInt << numDbl <<
                   text << numInt << numDbl);
    logTime = timer.Stop();

// print stats
    WRITE(std::setprecision(5) << std::left
        << std::setw(10) << "STREAM:" << logTime << " s" << std::endl);
}
