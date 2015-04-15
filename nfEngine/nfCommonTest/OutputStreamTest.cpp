#include "stdafx.hpp"
#include "../nfCommon/OutputStream.hpp"


namespace {
    const std::string text = "The quick brown fox jumps over the lazy dog";
    const std::string path = "samplefile.file";
} // namespace


class FileOutputStreamTest : public testing::Test
{
};

class BufferOutputStreamTest : public testing::Test
{
};


TEST_F(FileOutputStreamTest, CreateFileTest)
{
    bool resultBefore = std::ifstream(path).good();
    auto stream = new NFE::Common::FileOutputStream(path.c_str());
    auto error = GetLastError();
    bool resultAfter = std::ifstream(path).good();

    EXPECT_EQ(resultBefore, false);
    ASSERT_EQ(resultAfter, true);
    ASSERT_EQ(error, 0);
}

TEST_F(FileOutputStreamTest, OverwriteFileTest)
{
    std::ofstream file;
    file.open(path);
    file << text;
    file.close();

    bool resultBefore = std::ifstream(path).good();
    auto stream = new NFE::Common::FileOutputStream(path.c_str());
    auto error = GetLastError();
    bool resultAfter = std::ifstream(path).good();

    ASSERT_EQ(resultBefore, resultAfter);
    ASSERT_EQ(error, ERROR_ALREADY_EXISTS);
    delete stream;

    file.open(path);
    stream = new NFE::Common::FileOutputStream(path.c_str());
    error = GetLastError();
    EXPECT_EQ(stream, INVALID_HANDLE_VALUE); // jak to sprawdzic?
    ASSERT_EQ(error, ERROR_ALREADY_EXISTS);
    file.close();
    delete stream;
}

TEST_F(FileOutputStreamTest, WriteFileTest)
{
    size_t textSize = static_cast<size_t>(text.length());
    auto stream = new NFE::Common::FileOutputStream(path.c_str());
    size_t writeResult = stream->Write(text.c_str(), textSize);
    ASSERT_EQ(textSize, writeResult);
    delete stream;

    std::ifstream ifs;
    ifs.open(path, std::ios::binary | std::ios::ate);
    size_t fileSize = static_cast<size_t>(ifs.tellg());
    ASSERT_EQ(writeResult, fileSize);
    ifs.close();

    ifs.open(path);
    std::string result;
    ifs >> result;
    ASSERT_EQ(text.compare(result), 0);
    ifs.close();
}


TEST_F(BufferOutputStreamTest, ConstructorTest)
{
    auto stream = new NFE::Common::BufferOutputStream();
    ASSERT_EQ(stream->GetSize(), static_cast<size_t>(0));
    ASSERT_EQ(stream->GetData(), nullptr);
    ASSERT_EQ(stream->Write(text.c_str(), 1), false);
}

TEST_F(BufferOutputStreamTest, WriteTest)
{
    size_t strSize = text.length();
    auto stream = new NFE::Common::BufferOutputStream();
    size_t writeResult = stream->Write(text.c_str(), strSize);
    size_t streamSize = static_cast<size_t>(stream->GetSize());
    const void* streamData = stream->GetData();
    ASSERT_EQ(streamSize, strSize);
    ASSERT_EQ(memcmp(streamData, &text, strSize), 0);

    writeResult = stream->Write(text.c_str(), strSize);
    streamSize = static_cast<size_t>(stream->GetSize());
    streamData = stream->GetData();
    std::string doubleStr = text + text;
    ASSERT_EQ(streamSize, 2*strSize);
    ASSERT_EQ(memcmp(streamData, &doubleStr, 2*strSize), 0);
}

TEST_F(BufferOutputStreamTest, ClearTest)
{
    size_t strSize = text.length();
    auto stream = new NFE::Common::BufferOutputStream();
    size_t writeResult = stream->Write(text.c_str(), strSize);
    size_t streamSize = static_cast<size_t>(stream->GetSize());
    const void* streamData = stream->GetData();
    ASSERT_EQ(streamSize, strSize);
    ASSERT_EQ(memcmp(streamData, &text, strSize), 0);

    stream->Clear();
    streamSize = static_cast<size_t>(stream->GetSize());
    streamData = stream->GetData();
    ASSERT_EQ(streamSize, 0);
    ASSERT_EQ(streamData, nullptr);
}