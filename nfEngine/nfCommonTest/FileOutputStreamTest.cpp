#include "stdafx.hpp"
#include "../nfCommon/OutputStream.hpp"


namespace {
    const std::string TEXT = "The quick brown fox jumps over the lazy dog";
    const std::string PATH = "sampleoutputfile.file";
} // namespace
using namespace NFE::Common;

class FileOutputStreamTest : public testing::Test
{
protected:
    void SetUp()
    {
        mStream = std::make_unique<FileOutputStream>(PATH.c_str());
        unsigned long error = GetLastError();
        ASSERT_EQ(error, 0); //i get error 183 all the time ;__;
    }
    std::unique_ptr<FileOutputStream> mStream;
};

TEST_F(FileOutputStreamTest, OverwriteFileTest)
{
    std::string path = "empty" + PATH;
    std::ofstream file;
    file.open(path);
    file << TEXT;
    file.close();
    
    FileOutputStream stream(path.c_str());
    unsigned long error = GetLastError();
    ASSERT_EQ(error, ERROR_ALREADY_EXISTS);

    path += "1";
    file.open(path);
    FileOutputStream streamBad(path.c_str());
    error = GetLastError();
    //EXPECT_EQ(streamBad, INVALID_HANDLE_VALUE); // jak to sprawdzic?
    ASSERT_EQ(error, ERROR_ALREADY_EXISTS);
    file.close();
}

TEST_F(FileOutputStreamTest, WriteFileTest)
{
    size_t textSize = static_cast<size_t>(TEXT.length());
    size_t writeResult = mStream->Write(TEXT.c_str(), textSize);
    ASSERT_EQ(textSize, writeResult);

    std::ifstream ifs;
    ifs.open(PATH, std::ios::binary | std::ios::ate);
    size_t fileSize = static_cast<size_t>(ifs.tellg());
    ASSERT_EQ(writeResult, fileSize);
    ifs.close();

    ifs.open(PATH);
    std::string result;
    ifs >> result;
    ASSERT_EQ(TEXT.compare(result), 0);
    ifs.close();
}