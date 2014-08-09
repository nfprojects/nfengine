#include "stdafx.h"
#include "../nfCommon/Packer/packer.hpp"

using namespace NFE::Common;

const std::string testPackFilePath = "testfile.nfp";

class PackerBasicTest : public testing::Test
{
protected:
    void SetUp()
    {
        PACK_RESULT ret;
        EXPECT_NO_THROW(ret = Packer_CreateReader(&reader));
        EXPECT_EQ(PACK_RESULT::OK, ret);
        EXPECT_NE(nullptr, reader);

        EXPECT_NO_THROW(ret = Packer_CreateWriter(&writer));
        EXPECT_EQ(PACK_RESULT::OK, ret);
        EXPECT_NE(nullptr, writer);
    }

    void TearDown()
    {
        EXPECT_NO_THROW(Packer_ReleaseReader());
        EXPECT_NO_THROW(Packer_ReleaseWriter());
        reader = nullptr;
        writer = nullptr;

        std::ifstream fs(testPackFilePath);
        if (fs.good())
        {
            fs.close();
            EXPECT_EQ(0, remove(testPackFilePath.c_str())) << "remove() failed. "
                    << "Error: " << errno << " (" << strerror(errno) << ")";
        }
    }

    PackReader* reader;
    PackWriter* writer;
};

TEST_F(PackerBasicTest, WriterInitTest)
{
    PACK_RESULT pr;
    EXPECT_NO_THROW(pr = writer->Init(testPackFilePath));
    EXPECT_EQ(PACK_RESULT::OK, pr);
}

TEST_F(PackerBasicTest, WriterEmptyTest)
{
    PACK_RESULT pr;
    EXPECT_NO_THROW(pr = writer->Init(testPackFilePath));
    EXPECT_EQ(PACK_RESULT::OK, pr);

    EXPECT_NO_THROW(pr = writer->WritePAK());
    EXPECT_EQ(PACK_RESULT::OK, pr);

    // open file manually and check if it contains data in order:
    //   * version number
    //   * number of files (in this case 0)
    std::ifstream fs(testPackFilePath);
    EXPECT_TRUE(fs.good());

    uint32 readFileVersion;
    size_t readFileCount;

    fs.read(reinterpret_cast<char*>(&readFileVersion), sizeof(uint32));
    fs.read(reinterpret_cast<char*>(&readFileCount), sizeof(size_t));

    EXPECT_EQ(gPackFileVersion, readFileVersion);
    EXPECT_EQ(0, readFileCount);
}

TEST_F(PackerBasicTest, ReaderEmptyTest)
{
    // create correct file
    PACK_RESULT pr;
    EXPECT_NO_THROW(pr = writer->Init(testPackFilePath));
    EXPECT_EQ(PACK_RESULT::OK, pr);

    EXPECT_NO_THROW(pr = writer->WritePAK());
    EXPECT_EQ(PACK_RESULT::OK, pr);

    // open it with reader
    EXPECT_NO_THROW(pr = reader->Init(testPackFilePath));
    EXPECT_EQ(PACK_RESULT::OK, pr);

    // get file version and file size
    uint32 readFileVersion;
    size_t readFileCount;
    EXPECT_NO_THROW(readFileVersion = reader->GetFileVersion());
    EXPECT_EQ(gPackFileVersion, readFileVersion);

    EXPECT_NO_THROW(readFileCount = reader->GetFileCount());
    EXPECT_EQ(0, readFileCount);
}

TEST_F(PackerBasicTest, ReaderBuggyPathTest)
{
    PACK_RESULT pr;

    // open path to non existing file with reader
    EXPECT_NO_THROW(pr = reader->Init(testPackFilePath));
    EXPECT_EQ(PACK_RESULT::FILE_NOT_FOUND, pr);
}

TEST_F(PackerBasicTest, ReaderBuggyFileTest)
{
    PACK_RESULT pr;

    // create file with file version only
    std::ofstream file(testPackFilePath);
    EXPECT_TRUE(file.good());

    file.write(reinterpret_cast<const char*>(&gPackFileVersion), sizeof(gPackFileVersion));
    file.close();

    // initialization should fail - no information about file count inside archive
    EXPECT_NO_THROW(pr = reader->Init(testPackFilePath));
    EXPECT_EQ(PACK_RESULT::READ_FAILED, pr);
}
