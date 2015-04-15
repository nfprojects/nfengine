#include "stdafx.hpp"
#include "../nfCommon/InputStream.hpp"


namespace {
    const std::string PATH = "samplefile.file";
    const std::string TEXT = "The quick brown fox jumps over the lazy dog";
} // namespace
using namespace NFE::Common;

class FileInputStreamTest : public testing::Test
{
protected:
    void SetUp()
    {
        std::ofstream file;
        file.open("samplefile.file");
        file << TEXT;
        file.close();
        mStream = std::make_unique<FileInputStream>(PATH.c_str());
        mStreamSize = static_cast<size_t>(mStream->GetSize());
    }
    std::unique_ptr<FileInputStream> mStream;
    size_t mStreamSize;
};

TEST_F(FileInputStreamTest, NonExistentFileTest)
{
    FileInputStream stream("samplefilenotexisting.file");
	auto error = GetLastError();
	ASSERT_EQ(ERROR_FILE_NOT_FOUND, error);
}

TEST_F(FileInputStreamTest, GetSizeStringTest)
{
    ASSERT_EQ(mStreamSize, TEXT.length());
}

TEST_F(FileInputStreamTest, GetSizeEmptyTest)
{
    std::ofstream file;
    std::string fileName = "empty" + PATH;
    file.open(fileName);
    file.close();
    
    FileInputStream stream(fileName.c_str());
	size_t streamSize = static_cast<size_t>(stream.GetSize());
    ASSERT_EQ(streamSize, 0);
}

TEST_F(FileInputStreamTest, ReadTest)
{
	char fileText[50];
    size_t readSize = mStream->Read(mStreamSize, fileText);
    ASSERT_EQ(mStreamSize, readSize);

	fileText[readSize] = '\0';
	std::string fileStr(fileText);
	ASSERT_EQ(TEXT.compare(fileStr), 0);
}

TEST_F(FileInputStreamTest, SeekTest)
{
    char fileText[50];
    std::string fileText2;
    
    bool seekResult = mStream->Seek(mStreamSize / 2);
	ASSERT_EQ(seekResult, true);

    mStream->Read(mStreamSize / 2 + 1, fileText);
    fileText[mStreamSize / 2 + 1] = '\0';
	std::string fileStr(fileText);
    fileText2 = TEXT.substr(mStreamSize / 2, std::string::npos);
    ASSERT_EQ(fileStr.compare(fileText2), 0);
}

TEST_F(FileInputStreamTest, SeekOutOfBoundsTest)
{
    ASSERT_EQ(true, mStream->Seek(mStreamSize * 2)); //KINDA STUPID I THINK ;__;
}