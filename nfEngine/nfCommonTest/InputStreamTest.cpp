#include "stdafx.hpp"
#include "../nfCommon/InputStream.hpp"


namespace {
const std::string text = "The quick brown fox jumps over the lazy dog";
const std::string path = "samplefile.file";
const std::string notExistingPath = "samplefilenotexisting.file";
} // namespace


class FileInputStreamTest : public testing::Test
{
};

class BufferInputStreamTest : public testing::Test
{
};


TEST_F(FileInputStreamTest, NonExistentFileTest)
{
    bool result = std::ifstream(notExistingPath).good();
    ASSERT_EQ(result, false);
    auto stream = new NFE::Common::FileInputStream(notExistingPath.c_str());
    auto error = GetLastError();
    ASSERT_EQ(ERROR_FILE_NOT_FOUND, error);
    delete stream;
}

TEST_F(FileInputStreamTest, GetSizeStringTest)
{
    std::ofstream file;
    file.open(path);
    file << text;
    file.close();

    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    auto stream = new NFE::Common::FileInputStream(path.c_str());
    size_t streamSize = static_cast<size_t>(stream->GetSize());
    size_t fileSize = static_cast<size_t>(ifs.tellg());
    ASSERT_EQ(streamSize, fileSize);
    delete stream;
}

TEST_F(FileInputStreamTest, GetSizeEmptyTest)
{
    std::ofstream file;
    file.open (path);
    file.close();

    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    auto stream = new NFE::Common::FileInputStream(path.c_str());
    size_t streamSize = static_cast<size_t>(stream->GetSize());
    size_t fileSize = static_cast<size_t>(ifs.tellg());
    ASSERT_EQ(streamSize, fileSize);
    delete stream;
}

TEST_F(FileInputStreamTest, ReadTest)
{
    std::ofstream file;
    file.open (path);
    file << text;
    file.close();

    char fileText[50];
    auto stream = new NFE::Common::FileInputStream(path.c_str());
    size_t streamSize = static_cast<size_t>(stream->GetSize());
    size_t readSize = stream->Read(streamSize, fileText);
    ASSERT_EQ(streamSize, readSize);

    fileText[readSize] = '\0';
    std::string fileStr(fileText);
    ASSERT_EQ(text.compare(fileStr), 0);
    delete stream;
}

TEST_F(FileInputStreamTest, SeekTest)
{
    std::ofstream file;
    file.open (path);
    file << text;
    file.close();

    char fileText[50];
    std::string fileText2;
    auto stream = new NFE::Common::FileInputStream(path.c_str());
    size_t fileSize = static_cast<size_t>(stream->GetSize());
    bool seekResult = stream->Seek(fileSize / 2);
    ASSERT_EQ(seekResult, true);

    stream->Read(fileSize/2+1, fileText);
    fileText[fileSize/2+1] = '\0';
    std::string fileStr(fileText);
    fileText2 = text.substr(fileSize/2, std::string::npos);
    ASSERT_EQ(fileStr.compare(fileText2), 0);
    delete stream;
}

TEST_F(FileInputStreamTest, SeekOutOfBoundsTest)
{
    std::ofstream file;
    file.open (path);
    file << text;
    file.close();

    auto stream = new NFE::Common::FileInputStream(path.c_str());
    size_t fileSize = static_cast<size_t>(stream->GetSize());
    ASSERT_EQ(true, stream->Seek(fileSize*2)); //TAK JEST ZROBIONE...KINDA STUPID I THINK ;__;
    delete stream;
}


TEST_F(BufferInputStreamTest, NullBufferTest)
{
    auto stream = new NFE::Common::BufferInputStream(nullptr, 13);
    ASSERT_EQ(stream->GetSize(), 13);
    ASSERT_EQ(stream->Seek(1), false);
    ASSERT_EQ(stream->Read(1, nullptr), false);
    delete stream;
}

TEST_F(BufferInputStreamTest, StringBufferTest)
{
    size_t strSize = text.length();
    auto stream = new NFE::Common::BufferInputStream(&text, strSize);
    ASSERT_EQ(stream->GetSize(), strSize);
    char charBuffer[50];
    ASSERT_EQ(stream->Read(strSize, charBuffer), strSize);
    ASSERT_EQ(memcmp(charBuffer, &text, strSize), 0);
    delete stream;
}

TEST_F(BufferInputStreamTest, StringBufferSeekTest)
{
    size_t strSize = static_cast<size_t>(text.length());
    auto stream = new NFE::Common::BufferInputStream(text.c_str(), strSize);
    ASSERT_EQ(static_cast<size_t>(stream->GetSize()), strSize);

    char charBuffer[50];
    bool seekResult = stream->Seek(strSize / 2);
    ASSERT_EQ(seekResult, true);

    size_t readResult = stream->Read(strSize / 2, charBuffer);
    ASSERT_EQ(readResult, strSize / 2);

    ASSERT_EQ(memcmp(charBuffer, &text[strSize/2], strSize/2), 0);
    delete stream;
}