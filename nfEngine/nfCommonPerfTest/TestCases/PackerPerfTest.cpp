#include "PCH.hpp"
#include "Test.hpp"

#include "nfCommon/Packer/Packer.hpp"
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"
#include "nfCommon/FileSystem/File.hpp"
#include "nfCommon/Utils/StringUtils.hpp"


using namespace NFE;
using namespace NFE::Common;

namespace {
const uint32 TEST_FILE_MAX_COUNT = 4096;
const uint32 TEST_FILE_MAX_DATA_COUNT = 16384;

const String TEST_PACK_PATH = "testfile.nfp";
const String TEST_SAMPLE_FILE_DIR = "./TestDir/";
const String TEST_SAMPLE_FILE_REC_DIR_PREFIX = "TestRecDir_";
const String TEST_SAMPLE_FILE_PREFIX = "samplefile_";
} // namespace

class Packer : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        if (FileSystem::GetPathType(TEST_SAMPLE_FILE_DIR) != PathType::Invalid)
        {
            ASSERT_TRUE(FileSystem::Remove(TEST_SAMPLE_FILE_DIR, true)) <<
                        "Failed to remove test directory " << TEST_SAMPLE_FILE_DIR.Str() <<
                        ", it might be used. Aborting.";
        }

        std::cout << "Generating " << TEST_FILE_MAX_COUNT << " files for test case... ";

        srand(static_cast<unsigned int>(time(nullptr)));

        // TODO consider situation where TEST_SAMPLE_FILE_DIR is already created
        EXPECT_TRUE(FileSystem::CreateDir(TEST_SAMPLE_FILE_DIR)) <<
                    "Failed to create directory for test files.";

        const String testFilePrefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;

        for (uint32 i = 0; i < TEST_FILE_MAX_COUNT; ++i)
        {
            const String path = testFilePrefix + ToString(i);
            File f(path, AccessMode::Write, true);
            EXPECT_TRUE(f.IsOpened()) << "Failed to open " << path.Str();

            // generate <1; TEST_FILE_MAX_DATA_COUNT> uint32 values to write
            uint32 dataCount = (rand() % TEST_FILE_MAX_DATA_COUNT) + 1;
            DynArray<uint32> buffer;
            buffer.Resize(dataCount);

            for (uint32 j = 0; j < dataCount; ++j)
            {
                buffer[j] = rand();
            }

            f.Write(buffer.Data(), buffer.Size() * sizeof(uint32));
        }

        // Files are created, however AddFilesRecursively needs separate directories

        // FIXME this creates useless copies of already created sample files.
        //       Consider creating symlinks inside directories
        std::cout << "DONE\nCreating directories for Recursive tests... ";

        for (uint32 i = 1; i <= TEST_FILE_MAX_COUNT; i<<=1)
        {
            const String testRecDir = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_REC_DIR_PREFIX + ToString(i);
            EXPECT_TRUE(FileSystem::CreateDir(testRecDir));

            for (uint32 j = 0; j < i; j++)
            {
                FileSystem::Copy(testFilePrefix + ToString(j),
                                 testRecDir + "/" + TEST_SAMPLE_FILE_PREFIX + ToString(j));
            }
        }

        std::cout << "DONE" << std::endl;
    }

    void SetUp()
    {
        std::cout << "File count | Add time" << std::endl;
    }

    static void TearDownTestCase()
    {
        std::cout << "Removing files... ";

        // remove sample files
        FileSystem::Remove(TEST_SAMPLE_FILE_DIR, true);

        // remove created archive
        FileSystem::Remove(TEST_PACK_PATH);

        std::cout << "DONE" << std::endl;
    }

    UniquePtr<PackerReader> mReader;
    UniquePtr<PackerWriter> mWriter;
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
        mWriter = MakeUniquePtr<PackerWriter>();
        EXPECT_NE(nullptr, mWriter.Get());

        pr = mWriter->Init(TEST_PACK_PATH);
        EXPECT_EQ(PackerResult::OK, pr);

        // test performance of AddFile only
        // we don't take into account time for construction of argument strings
        const String prefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;
        String path;
        for (uint32 j = 0; j < i; ++j)
        {
            path = prefix + ToString(j);
            mTimer.Start();
            pr = mWriter->AddFile(path, path);
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
        mWriter = MakeUniquePtr<PackerWriter>();
        EXPECT_NE(nullptr, mWriter.Get());

        pr = mWriter->Init(TEST_PACK_PATH);
        EXPECT_EQ(PackerResult::OK, pr);

        // test performance of AddFilesRecursively
        const String testRecDir = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_REC_DIR_PREFIX + ToString(i);
        mTimer.Start();
        pr = mWriter->AddFilesRecursively(testRecDir);
        totalTime += mTimer.Stop();
        EXPECT_EQ(PackerResult::OK, pr);

        std::cout << totalTime << std::endl;
    }
}
