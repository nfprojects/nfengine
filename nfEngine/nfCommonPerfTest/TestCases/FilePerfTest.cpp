/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Performance tests for File related classes.
 */

#include "PCH.hpp"
#include "Test.hpp"
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/FileSystem/FileBuffered.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"
#include "nfCommon/Containers/String.hpp"


using namespace NFE::Common;

const int OPERATION_TEST_NUMBER = 50000;
const int BIG_TEXT_SIZE = 10000;

class FilePerf : public CommonPerfTest
{
    const std::string GetName() const override
    {
        return std::string("FilePerfTest");
    }

public:
    const char* mPath = "testFile.perf";

    void TearDown() override
    {
        // Clean up after tests
        FileSystem::Remove(mPath);
    }
};

TEST_F(FilePerf, SmallWrite)
{
    GetOStream() << "SmallWrite:" << std::endl;

    // Declare vars
    Timer timer;
    double fileTime, fileBuffTime;

    // Prepare the test data
    const char* data = "c";

    // Start testing File class
    File file(mPath, AccessMode::Write, true);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        file.Write(data, 1);
    fileTime = timer.Stop();

    file.Close();

    // Start testing FileBuffered class
    FileBuffered fileBuf(mPath, AccessMode::Write, true);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        fileBuf.Write(data, 1);
    fileBuffTime = timer.Stop();

    fileBuf.Close();

    // Print stats
    GetOStream() << std::setprecision(5) << std::left
        << std::setw(10) << "FILE:" << fileTime << " s" << std::endl
        << std::setw(10) << "FILEBUFF:" << fileBuffTime << " s" << std::endl << std::endl;
}

TEST_F(FilePerf, BigWrite)
{
    GetOStream() << "BigWrite:" << std::endl;

    // Declare vars
    Timer timer;
    double fileTime, fileBuffTime;

    // Prepare the test data
    const std::string data(BIG_TEXT_SIZE, 'c');

    // Start testing File class
    File file(mPath, AccessMode::Write, true);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        file.Write(data.data(), BIG_TEXT_SIZE);
    fileTime = timer.Stop();

    file.Close();

    // Start testing FileBuffered class
    FileBuffered fileBuf(mPath, AccessMode::Write, true);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        fileBuf.Write(data.data(), BIG_TEXT_SIZE);
    fileBuffTime = timer.Stop();

    fileBuf.Close();

    // Print stats
    GetOStream() << std::setprecision(5) << std::left
        << std::setw(10) << "FILE:" << fileTime << " s" << std::endl
        << std::setw(10) << "FILEBUFF:" << fileBuffTime << " s" << std::endl << std::endl;
}

TEST_F(FilePerf, VariousWrite)
{
    GetOStream() << "VariousWrite:" << std::endl;

    // Declare vars
    Timer timer;
    double fileTime, fileBuffTime;

    // Prepare the test data
    const std::string data(OPERATION_TEST_NUMBER, 'c');

    // Start testing File class
    File file(mPath, AccessMode::Write, true);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        file.Write(data.data(), i);
    fileTime = timer.Stop();

    file.Close();

    // Start testing FileBuffered class
    FileBuffered fileBuf(mPath, AccessMode::Write, true);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        fileBuf.Write(data.data(), i);
    fileBuffTime = timer.Stop();

    fileBuf.Close();

    // Print stats
    GetOStream() << std::setprecision(5) << std::left
        << std::setw(10) << "FILE:" << fileTime << " s" << std::endl
        << std::setw(10) << "FILEBUFF:" << fileBuffTime << " s" << std::endl << std::endl;
}

TEST_F(FilePerf, SmallRead)
{
    GetOStream() << "SmallRead:" << std::endl;

    // Declare vars
    Timer timer;
    char buffer;
    double fileTime, fileBuffTime;

    // Prepare the test data
    const std::string data(OPERATION_TEST_NUMBER, 'c');
    File filePrep(mPath, AccessMode::Write, true);
    filePrep.Write(data.data(), OPERATION_TEST_NUMBER);
    filePrep.Close();

    // Start testing File class
    File file(mPath, AccessMode::Read);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        file.Read(&buffer, 1);
    fileTime = timer.Stop();

    file.Close();

    // Start testing FileBuffered class
    FileBuffered fileBuf(mPath, AccessMode::Read);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        fileBuf.Read(&buffer, 1);
    fileBuffTime = timer.Stop();

    fileBuf.Close();

    // Print stats
    GetOStream() << std::setprecision(5) << std::left
        << std::setw(10) << "FILE:" << fileTime << " s" << std::endl
        << std::setw(10) << "FILEBUFF:" << fileBuffTime << " s" << std::endl << std::endl;
}

TEST_F(FilePerf, BigRead)
{
    GetOStream() << "BigRead:" << std::endl;

    // Declare vars
    Timer timer;
    char buffer[BIG_TEXT_SIZE];
    double fileTime, fileBuffTime;

    // Prepare the test data
    const std::string data(BIG_TEXT_SIZE * OPERATION_TEST_NUMBER, 'c');
    File filePrep(mPath, AccessMode::Write, true);
    filePrep.Write(data.data(), BIG_TEXT_SIZE * OPERATION_TEST_NUMBER);
    filePrep.Close();

    // Start testing File class
    File file(mPath, AccessMode::Read);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        file.Read(buffer, BIG_TEXT_SIZE);
    fileTime = timer.Stop();

    file.Close();

    // Start testing FileBuffered class
    FileBuffered fileBuf(mPath, AccessMode::Read);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        fileBuf.Read(buffer, BIG_TEXT_SIZE);
    fileBuffTime = timer.Stop();

    fileBuf.Close();

    // Print stats
    GetOStream() << std::setprecision(5) << std::left
        << std::setw(10) << "FILE:" << fileTime << " s" << std::endl
        << std::setw(10) << "FILEBUFF:" << fileBuffTime << " s" << std::endl << std::endl;
}

TEST_F(FilePerf, VariousRead)
{
    GetOStream() << "VariousRead:" << std::endl;

    // Declare vars
    Timer timer;
    char buffer[OPERATION_TEST_NUMBER];
    double fileTime, fileBuffTime;

    // Prepare the test data
    size_t bufferSize = OPERATION_TEST_NUMBER + 1;
    bufferSize *= (OPERATION_TEST_NUMBER / 2);
    const std::string data(bufferSize, 'c');
    File filePrep(mPath, AccessMode::Write, true);
    filePrep.Write(data.data(), bufferSize);
    filePrep.Close();

    // Start testing File class
    File file(mPath, AccessMode::Read);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        file.Read(buffer, i);
    fileTime = timer.Stop();

    file.Close();

    // Start testing FileBuffered class
    FileBuffered fileBuf(mPath, AccessMode::Read);

    timer.Start();
    for (int i = OPERATION_TEST_NUMBER; i > 0; i--)
        fileBuf.Read(buffer, i);
    fileBuffTime = timer.Stop();

    fileBuf.Close();

    // Print stats
    GetOStream() << std::setprecision(5) << std::left
        << std::setw(10) << "FILE:" << fileTime << " s" << std::endl
        << std::setw(10) << "FILEBUFF:" << fileBuffTime << " s" << std::endl << std::endl;
}
