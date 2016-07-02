/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Performance tests for Logger class.
 */

#include "PCH.hpp"
#include "nfCommon/Timer.hpp"
#include "Test.hpp"


using namespace NFE::Common;

const int LOG_TEST_NUMBER = 10000;

class LoggerPerf : public CommonPerfTest
{
    const std::string GetName() const override
    {
        return std::string("LoggerPerfTest");
    }

public:
    void TestFixture(std::function<void()> testFunc)
    {
        auto bckndList = Logger::ListBackends();
        for (const auto& b : bckndList)
            Logger::GetBackend(b)->Enable(false);

        for (const auto& b : bckndList)
        {
            auto backend = Logger::GetBackend(b);
            backend->Enable(true);
            GetOStream() << std::endl << "<>LOGGER BACKEND: " << b << std::endl;

            testFunc();

            backend->Enable(false);
        }
    }
};

TEST_F(LoggerPerf, SimpleChars)
{
    TestFixture([this](){
        GetOStream() << "SimpleChars:" << std::endl;
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
        GetOStream() << std::setprecision(5) << std::left
            << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
            << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl;
    });
}

TEST_F(LoggerPerf, SymbolsShort)
{
    TestFixture([this](){
        GetOStream() << "SymbolsShort:" << std::endl;
        Timer timer;
        const std::string text = "<>'\"&";
        double logTime[5], logStreamTime[5];
        unsigned char counter = 0;

        for (const auto& i : text)
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

        GetOStream() << "Symbol | LOG_* time | LOG_*_S time" << std::endl;
        for (int i = 0; i < counter; i++)
        {
            // print stats
            GetOStream() << std::setprecision(5) << std::left
                << std::setw(6) << text[i] << " | "
                << std::setw(8) << logTime[i] << " s | "
                << std::setw(10) << logStreamTime[i] << " s" << std::endl;
        }
    });
}

TEST_F(LoggerPerf, SymbolsLong)
{
    TestFixture([this](){
        GetOStream() << "SymbolsLong:" << std::endl;
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
        GetOStream() << std::setprecision(5) << std::left
            << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
            << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl;
    });
}

TEST_F(LoggerPerf, LongText)
{
    TestFixture([this](){
        GetOStream() << "LongText:" << std::endl;
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
        GetOStream() << std::setprecision(5) << std::left
            << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl
            << std::setw(10) << "STREAM:" << logStreamTime << " s" << std::endl;
    });
}

TEST_F(LoggerPerf, FormattedMsg)
{
    TestFixture([this](){
        GetOStream() << "FormattedMsg:" << std::endl;
        Timer timer;
        double logTime;

        const int numInt = 5;
        const double numDbl = 5.55;
        const char* text = "Lorem ipsum";
        const char* format = "%s%i%f%s%i%f%s%i%f";

        timer.Start();
        for (int i = LOG_TEST_NUMBER; i > 0; i--)
            LOG_INFO(format, text, numInt, numDbl,
                     text, numInt, numDbl,
                     text, numInt, numDbl);
        logTime = timer.Stop();

         // print stats
        GetOStream() << std::setprecision(5) << std::left
            << std::setw(10) << "NORMAL:" << logTime << " s" << std::endl;
    });
}

TEST_F(LoggerPerf, StreamMsg)
{
    TestFixture([this](){
        GetOStream() << "StreamMsg:" << std::endl;
        Timer timer;
        double logTime;

        const int numInt = 5;
        const double numDbl = 5.55;
        const char* text = "Lorem ipsum";

        timer.Start();
        for (int i = LOG_TEST_NUMBER; i > 0; i--)
            LOG_INFO_S(text << numInt << numDbl <<
                       text << numInt << numDbl <<
                       text << numInt << numDbl);
        logTime = timer.Stop();

        // print stats
        GetOStream() << std::setprecision(5) << std::left
            << std::setw(10) << "STREAM:" << logTime << " s" << std::endl;
    });
}
