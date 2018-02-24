#include "PCH.hpp"
#include "nfCommon/Packer/Packer.hpp"



using namespace NFE;
using namespace NFE::Common;

const String testPackFilePath("testfile.nfp");

class PackerBasicTest : public testing::Test
{
protected:
    void SetUp()
    {
        mReader.reset(new PackerReader());
        EXPECT_NE(nullptr, mReader.get());

        mWriter.reset(new PackerWriter());
        EXPECT_NE(nullptr, mWriter.get());
    }

    void TearDown()
    {
        mReader.reset();
        mWriter.reset();

        FileSystem::Remove(testPackFilePath);
    }

    std::unique_ptr<PackerReader> mReader;
    std::unique_ptr<PackerWriter> mWriter;
};

TEST_F(PackerBasicTest, WriterInitTest)
{
    PackerResult pr;
    pr = mWriter->Init(testPackFilePath);
    EXPECT_EQ(PackerResult::OK, pr);
}

TEST_F(PackerBasicTest, WriterEmptyTest)
{
    PackerResult pr;
    pr = mWriter->Init(testPackFilePath);
    EXPECT_EQ(PackerResult::OK, pr);

    pr = mWriter->WritePAK();
    EXPECT_EQ(PackerResult::OK, pr);

    // open file manually and check if it contains data in order:
    //   * version number
    //   * number of files (in this case 0)
    std::ifstream fs(testPackFilePath.Str());
    EXPECT_TRUE(fs.good());

    uint32 readFileVersion;
    uint32 readFileCount;

    fs.read(reinterpret_cast<char*>(&readFileVersion), sizeof(uint32));
    fs.read(reinterpret_cast<char*>(&readFileCount), sizeof(uint32));

    EXPECT_EQ(gPackFileVersion, readFileVersion);
    EXPECT_EQ(0, readFileCount);
}

TEST_F(PackerBasicTest, ReaderEmptyTest)
{
    // create correct file
    PackerResult pr;
    pr = mWriter->Init(testPackFilePath);
    EXPECT_EQ(PackerResult::OK, pr);

    pr = mWriter->WritePAK();
    EXPECT_EQ(PackerResult::OK, pr);

    // open it with reader
    pr = mReader->Init(testPackFilePath);
    EXPECT_EQ(PackerResult::OK, pr);

    // get file version and file size
    uint32 readFileVersion;
    size_t readFileCount;
    readFileVersion = mReader->GetFileVersion();
    EXPECT_EQ(gPackFileVersion, readFileVersion);

    readFileCount = mReader->GetFileCount();
    EXPECT_EQ(0, readFileCount);
}

TEST_F(PackerBasicTest, ReaderBuggyPathTest)
{
    PackerResult pr;

    // open path to non existing file with reader
    pr = mReader->Init(testPackFilePath);
    EXPECT_EQ(PackerResult::FileNotFound, pr);
}

TEST_F(PackerBasicTest, ReaderBuggyFileTest)
{
    PackerResult pr;

    // create file with file version only
    std::ofstream file(testPackFilePath.Str());
    EXPECT_TRUE(file.good());

    file.write(reinterpret_cast<const char*>(&gPackFileVersion), sizeof(gPackFileVersion));
    file.close();

    // initialization should fail - no information about file count inside archive
    pr = mReader->Init(testPackFilePath);
    EXPECT_EQ(PackerResult::ReadFailed, pr);
}
