/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Performance tests for Logger class.
 */

#include "PCH.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/Timer.hpp"

using namespace NFE::Common;

const int LOG_TEST_NUMBER = 10000;

TEST(Logger, SimpleChars)
{
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
    std::cout << std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
        << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl;
}

TEST(Logger, SymbolsShort)
{
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

    std::cout << "Symbol | LOG_* time | LOG_*_S time" << std::endl;
    for (int i = 0; i < counter; i++)
    {
        // print stats
        std::cout << std::setprecision(5) << std::left
            << std::setw(6) << text[i] << " | "
            << std::setw(8) << logTime[i] << " s | "
            << std::setw(10) << logStreamTime[i] << " s" << std::endl;
    }
}

TEST(Logger, SymbolsLong)
{
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
    std::cout << std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
        << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl;
}

TEST(Logger, LongText)
{
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
    std::cout << std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
        << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl;
}

TEST(Logger, FormattedMsg)
{
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
    std::cout << std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl;
}

TEST(Logger, StreamMsg)
{
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
    std::cout << std::setprecision(5) << std::left
        << std::setw(10) << "STREAM:" << logTime << " s" << std::endl;
}