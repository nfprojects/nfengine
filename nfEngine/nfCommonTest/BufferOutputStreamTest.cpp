#include "stdafx.hpp"
#include "../nfCommon/OutputStream.hpp"

using namespace NFE::Common;

namespace {
    const std::string TEXT = "The quick brown fox jumps over the lazy dog";
    const std::string PATH = "sampleoutputfile.file";
} // namespace


class BufferOutputStreamTest : public testing::Test
{
protected:
    std::unique_ptr<BufferOutputStream> mStream;
    size_t mTextSize;

    void SetUp()
    {
        mStream = std::make_unique<BufferOutputStream>();
        mTextSize = static_cast<size_t>(TEXT.length());
    }
};


TEST_F(BufferOutputStreamTest, ConstructorTest)
{
    ASSERT_EQ(mStream->GetSize(), 0);
    ASSERT_EQ(mStream->GetData(), nullptr);
    ASSERT_EQ(mStream->Write(TEXT.c_str(), 1), 1);
}

TEST_F(BufferOutputStreamTest, WriteTest)
{
    mStream->Write(TEXT.c_str(), mTextSize);
    size_t streamSize = static_cast<size_t>(mStream->GetSize());
    const void* streamData = mStream->GetData();
    ASSERT_EQ(streamSize, mTextSize);

    ASSERT_EQ(memcmp(streamData, &TEXT, mTextSize), 0);

    streamSize = static_cast<size_t>(mStream->GetSize());
    streamData = mStream->GetData();
    std::string doubleStr = TEXT + TEXT;
    ASSERT_EQ(streamSize, 2 * mTextSize);
    ASSERT_EQ(memcmp(streamData, &doubleStr, 2 * mTextSize), 0);
}

TEST_F(BufferOutputStreamTest, ClearTest)
{
    size_t writeResult = mStream->Write(TEXT.c_str(), mTextSize);

    mStream->Clear();
    size_t streamSize = static_cast<size_t>(mStream->GetSize());
    const void* streamData = mStream->GetData();
    ASSERT_EQ(streamSize, 0);
    ASSERT_EQ(streamData, nullptr);
}