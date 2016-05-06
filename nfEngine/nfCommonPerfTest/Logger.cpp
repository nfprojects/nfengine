/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Performance tests for Logger class.
 */

#include "PCH.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/Timer.hpp"

using namespace NFE::Common;

TEST(Logger, SimpleChars)
{
    Timer timer;
    const char* simple = "c";
    double logTime, logStreamTime;

    timer.Start();
    LOG_WARNING(simple);
    logTime = 1000 * timer.Stop();

    timer.Start();
    LOG_WARNING_S(simple);
    logStreamTime = 1000 * timer.Stop();

    // print stats
    std::cout << std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << std::endl
        << std::setw(10) << "STREAM:" << logStreamTime << std::endl;

    getchar();
}

TEST(Logger, SymbolsShort)
{
    Timer timer;
    std::string text = "<>'\"&";
    double logTime[5], logStreamTime[5];
    unsigned char counter = 0;

    for (auto& i : text)
    {
        timer.Start();
        LOG_WARNING(std::to_string(i).c_str());
        logTime[counter] = 1000 * timer.Stop();

        timer.Start();
        LOG_WARNING_S(std::to_string(i).c_str());
        logStreamTime[counter++] = 1000 * timer.Stop();
    }

    std::cout << "Symbol | LOG_* time | LOG_*_S time" << std::endl;
    for (int i = 0; i < counter; i++)
    {
        // print stats
        std::cout << std::setprecision(5) << std::left
            << std::setw(6) << text[i] << " | "
            << std::setw(10) << logTime[i] << " | "
            << std::setw(12) << logStreamTime[i] << std::endl;
    }

    getchar();
}

TEST(Logger, SymbolsLong)
{
    Timer timer;
    const char* text = "~!@#$%^&*()_+=-<?.';][\\,>/:\"{}|";
    double logTime, logStreamTime;

    timer.Start();
    LOG_WARNING(text);
    logTime = 1000 * timer.Stop();

    timer.Start();
    LOG_WARNING_S(text);
    logStreamTime = 1000 * timer.Stop();

    // print stats
    std::cout << std::setprecision(5) << std::left
        << std::setw(10) << "NORMAL:" << logTime << std::endl
        << std::setw(10) << "STREAM:" << logStreamTime << std::endl;

    getchar();
}

TEST(Logger, VariousLengthText)
{
    Timer timer;
    std::string text = "text";
    double logTime[16], logStreamTime[16];
    unsigned char counter = 0;
    unsigned int size[16];

    for (int i = 0; i < 15; i++)
    {
        timer.Start();
        LOG_WARNING(text.c_str());
        logTime[counter] = 1000 * timer.Stop();

        timer.Start();
        LOG_WARNING_S(text.c_str());
        logStreamTime[counter] = 1000 * timer.Stop();

        size[counter++] = static_cast<unsigned int>(text.size());
        text += text;
    }

    std::cout << "String size | LOG_* time | LOG_*_S time" << std::endl;
    for (int i = 0; i < counter; i++)
    {
        // print stats
        std::cout << std::setprecision(5) << std::left
            << std::setw(11) << size[i] << " | "
            << std::setw(10) << logTime[i] << " | "
            << std::setw(12) << logStreamTime[i] << std::endl;
    }

    getchar();
}
