#include "stdafx.hpp"
#include "Packer/Packer.hpp"
#include "Timer.hpp"

#include "test.hpp"

using namespace NFE::Common;

namespace {
const uint32 TEST_FILE_MAX_COUNT = 4096;
const uint32 TEST_FILE_MAX_DATA_COUNT = 16384;

const std::string TEST_PACK_PATH = "testfile.nfp";
const std::string TEST_SAMPLE_FILE_DIR = "./TestDir/";
const std::string TEST_SAMPLE_FILE_PREFIX = "samplefile_";
} // namespace

class Packer : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        std::cout << "Generating " << TEST_FILE_MAX_COUNT << " files for test case... ";

        srand(static_cast<unsigned int>(time(nullptr)));

        int result = mkdir(TEST_SAMPLE_FILE_DIR.c_str());
        EXPECT_FALSE(result != 0 && errno != EEXIST) << "mkdir() failed. "
                << "Error: " << errno << " (" << strerror(errno) << ")";

        const std::string testFilePrefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;

        for (uint32 i = 0; i < TEST_FILE_MAX_COUNT; ++i)
        {
            std::ofstream fs(testFilePrefix + std::to_string(i),
                             std::fstream::out | std::fstream::trunc | std::fstream::binary);
            EXPECT_TRUE(fs.good()) << "Failed to open " << TEST_SAMPLE_FILE_DIR
                                   << TEST_SAMPLE_FILE_PREFIX << std::to_string(i);

            // generate <1; TEST_FILE_MAX_DATA_COUNT> uint32 values to write
            uint32 dataCount = (rand() % TEST_FILE_MAX_DATA_COUNT) + 1;
            for (uint32 j = 0; j < dataCount; ++j)
            {
                uint32 val = rand();
                fs.write(reinterpret_cast<const char*>(&val), sizeof(uint32));
            }
        }

        std::cout << "DONE" << std::endl;
    }

    void SetUp()
    {
        std::cout << "File count | Result time" << std::endl;
    }

    static void TearDownTestCase()
    {
        std::cout << "Removing files... ";

        // remove sample files
        const std::string prefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;
        std::string path;
        for (uint32 i = 0; i < TEST_FILE_MAX_COUNT; ++i)
        {
            path = prefix + std::to_string(i);
            EXPECT_EQ(0, remove(path.c_str())) << "remove(" << path.c_str() << ") failed. "
                                               << "Error: " << errno << " (" << strerror(errno) << ")";
        }

        rmdir(TEST_SAMPLE_FILE_DIR.c_str());

        // remove created archive
        std::ifstream fs(TEST_PACK_PATH);
        if (fs.good())
        {
            fs.close();
            EXPECT_EQ(0, remove(TEST_PACK_PATH.c_str())) << "remove() failed. "
                    << "Error: " << errno << " (" << strerror(errno) << ")";
        }

        std::cout << "DONE" << std::endl;
    }

    std::unique_ptr<PackerReader> mReader;
    std::unique_ptr<PackerWriter> mWriter;
    Timer mTimer;
};

TEST_F(Packer, AddFile)
{
    PackerResult pr;
    double totalTime;

    for (uint32 i = 2; i <= TEST_FILE_MAX_COUNT; i<<=1)
    {
        std::cout << std::setw(10) << i << " | ";

        totalTime = 0.0;

        // reset writer
        EXPECT_NO_THROW(mWriter.reset(new PackerWriter()));
        EXPECT_NE(nullptr, mWriter.get());

        EXPECT_NO_THROW(pr = mWriter->Init(TEST_PACK_PATH));
        EXPECT_EQ(PackerResult::OK, pr);

        // test performance of AddFile only
        // we don't take into account time for construction of argument strings
        const std::string prefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;
        std::string path;
        for (uint32 j = 0; j < i; ++j)
        {
            path = prefix + std::to_string(j);
            mTimer.Start();
            EXPECT_NO_THROW(pr = mWriter->AddFile(path, path));
            totalTime += mTimer.Stop();
            EXPECT_EQ(PackerResult::OK, pr);
        }

        std::cout << totalTime << std::endl;
    }
}

TEST_F(Packer, AddFilesRecursively)
{
    PackerResult pr;
    double totalTime;

    for (uint32 i = 2; i <= TEST_FILE_MAX_COUNT; i<<=1)
    {
        std::cout << std::setw(10) << i << " | ";

        totalTime = 0.0;

        // reset writer
        EXPECT_NO_THROW(mWriter.reset(new PackerWriter()));
        EXPECT_NE(nullptr, mWriter.get());

        EXPECT_NO_THROW(pr = mWriter->Init(TEST_PACK_PATH));
        EXPECT_EQ(PackerResult::OK, pr);

        // test performance of AddFilesRecursively
        mTimer.Start();
        EXPECT_NO_THROW(pr = mWriter->AddFilesRecursively(TEST_SAMPLE_FILE_DIR));
        totalTime += mTimer.Stop();
        EXPECT_EQ(PackerResult::OK, pr);

        std::cout << totalTime << std::endl;
    }
}
