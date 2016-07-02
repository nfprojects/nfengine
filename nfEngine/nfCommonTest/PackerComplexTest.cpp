#include "PCH.hpp"
#include "nfCommon/Packer/Packer.hpp"

#include "Constants.hpp"

#include <fstream>


using namespace NFE::Common;

namespace {
const unsigned int TEST_FILE_COUNT = 1000; // amount of random files to generate for testing
const unsigned int TEST_FILE_MAX_DATA_COUNT =
    8192; // maximum amount of random data to put inside files
} // namespace

class PackerComplexTest : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        std::cout << "Generating " << TEST_FILE_COUNT << " files for test case... ";

        srand(static_cast<unsigned int>(time(nullptr)));

        int result = _mkdir(TEST_SAMPLE_FILE_DIR.c_str());
        EXPECT_FALSE(result != 0 && errno != EEXIST) << "mkdir() failed. "
                << "Error: " << errno << " (" << strerror(errno) << ")";

        const std::string testFilePrefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;

        for (uint32 i = 0; i < TEST_FILE_COUNT; ++i)
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
        EXPECT_NO_THROW(mReader.reset(new PackerReader()));
        EXPECT_NE(nullptr, mReader.get());

        EXPECT_NO_THROW(mWriter.reset(new PackerWriter()));
        EXPECT_NE(nullptr, mWriter.get());
    }

    void TearDown()
    {
        mReader.reset();
        mWriter.reset();
    }

    static void TearDownTestCase()
    {
        std::cout << "Removing files... ";

        // remove sample files
        const std::string prefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;
        std::string path;
        for (uint32 i = 0; i < TEST_FILE_COUNT; ++i)
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
};

TEST_F(PackerComplexTest, AddFiles)
{
    PackerResult pr;

    // initialize mWriter
    EXPECT_NO_THROW(pr = mWriter->Init(TEST_PACK_PATH));
    EXPECT_EQ(PackerResult::OK, pr);

    // add files
    const std::string prefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;
    std::string path;
    for (int i = 0; i < TEST_FILE_COUNT; ++i)
    {
        path = prefix + std::to_string(i);
        EXPECT_NO_THROW(pr = mWriter->AddFile(path, path));
        ASSERT_EQ(PackerResult::OK, pr);
    }
}

TEST_F(PackerComplexTest, AddFilesRecursively)
{
    PackerResult pr;

    // initialize mWriter
    EXPECT_NO_THROW(pr = mWriter->Init(TEST_PACK_PATH));
    EXPECT_EQ(PackerResult::OK, pr);

    // add files recursively
    EXPECT_NO_THROW(pr = mWriter->AddFilesRecursively(TEST_SAMPLE_FILE_DIR));
    ASSERT_EQ(PackerResult::OK, pr);
}

TEST_F(PackerComplexTest, WriteFiles)
{
    PackerResult pr;

    // initialize mWriter
    EXPECT_NO_THROW(pr = mWriter->Init(TEST_PACK_PATH));
    EXPECT_EQ(PackerResult::OK, pr);

    // add files recursively
    EXPECT_NO_THROW(pr = mWriter->AddFilesRecursively(TEST_SAMPLE_FILE_DIR));
    EXPECT_EQ(PackerResult::OK, pr);

    // write files to PAK archive
    EXPECT_NO_THROW(pr = mWriter->WritePAK());
    ASSERT_EQ(PackerResult::OK, pr);
}

TEST_F(PackerComplexTest, ReadSingleFile)
{
    PackerResult pr;
    const std::string testFilePathVFS = TEST_SAMPLE_FILE_PREFIX + "0";
    const std::string testFilePath = TEST_SAMPLE_FILE_DIR + testFilePathVFS;

    // initialize mWriter
    EXPECT_NO_THROW(pr = mWriter->Init(TEST_PACK_PATH));
    EXPECT_EQ(PackerResult::OK, pr);

    // add files recursively
    EXPECT_NO_THROW(pr = mWriter->AddFile(testFilePath, testFilePathVFS));
    EXPECT_EQ(PackerResult::OK, pr);

    // write files to PAK archive
    EXPECT_NO_THROW(pr = mWriter->WritePAK());
    ASSERT_EQ(PackerResult::OK, pr);

    // initialize mReader
    EXPECT_NO_THROW(pr = mReader->Init(TEST_PACK_PATH));
    ASSERT_EQ(PackerResult::OK, pr);

    // check file count
    size_t fileCount;
    EXPECT_NO_THROW(fileCount = mReader->GetFileCount());
    ASSERT_EQ(1, fileCount);

    std::unique_ptr<Buffer> readData, correctData;
    readData.reset(new Buffer());
    correctData.reset(new Buffer());

    // read file from PAK
    EXPECT_NO_THROW(pr = mReader->GetFile(testFilePathVFS, *readData));
    ASSERT_EQ(PackerResult::OK, pr);

    // read original file
    FILE* pFile = fopen(testFilePath.c_str(), "rb");
    ASSERT_NE(nullptr, pFile);

    // get file size
    fseek(pFile, 0, SEEK_END);
    uint32 correctDataSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    // create buffer and read data
    correctData->Create(correctDataSize);
    ASSERT_EQ(fread(correctData->GetData(), sizeof(unsigned char), correctDataSize, pFile),
              correctDataSize);
    fclose(pFile);

    // check data size
    ASSERT_EQ(correctDataSize, readData->GetSize());

    // compare data
    ASSERT_EQ(0, memcmp(readData->GetData(), correctData->GetData(), correctDataSize));
}

TEST_F(PackerComplexTest, ReadMultipleSimplyAddedFiles)
{
    PackerResult pr;

    // initialize mWriter
    EXPECT_NO_THROW(pr = mWriter->Init(TEST_PACK_PATH));
    EXPECT_EQ(PackerResult::OK, pr);

    // add files through simple AddFile function
    const std::string prefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;
    std::string path;
    for (int i = 0; i < TEST_FILE_COUNT; ++i)
    {
        path = prefix + std::to_string(i);
        EXPECT_NO_THROW(pr = mWriter->AddFile(path, path));
        ASSERT_EQ(PackerResult::OK, pr);
    }

    // write files to PAK archive
    EXPECT_NO_THROW(pr = mWriter->WritePAK());
    ASSERT_EQ(PackerResult::OK, pr);

    // initialize mReader
    EXPECT_NO_THROW(pr = mReader->Init(TEST_PACK_PATH));
    ASSERT_EQ(PackerResult::OK, pr);

    // check file count
    size_t fileCount;
    EXPECT_NO_THROW(fileCount = mReader->GetFileCount());
    ASSERT_EQ(TEST_FILE_COUNT, fileCount);

    std::unique_ptr<Buffer> readData, correctData;
    // read every file and see if data was copied correctly
    for (int i = 0; i < TEST_FILE_COUNT; ++i)
    {
        //initialize loop iteration
        path = prefix + std::to_string(i);
        readData.reset(new Buffer());
        correctData.reset(new Buffer());

        // read file from archive
        EXPECT_NO_THROW(pr = mReader->GetFile(path, *readData)) << "i = " << i;
        ASSERT_EQ(PackerResult::OK, pr) << "i = " << i;

        // read original file
        FILE* pFile = fopen(path.c_str(), "rb");
        ASSERT_NE(nullptr, pFile) << "i = " << i;

        // get file size
        fseek(pFile, 0, SEEK_END);
        uint32 correctDataSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        // create buffer and read data
        correctData->Create(correctDataSize);
        ASSERT_EQ(fread(correctData->GetData(), sizeof(unsigned char), correctDataSize, pFile),
                  correctDataSize);
        fclose(pFile);

        // check data size
        ASSERT_EQ(correctDataSize, readData->GetSize()) << "i = " << i;

        // compare data
        ASSERT_EQ(0, memcmp(readData->GetData(), correctData->GetData(), correctDataSize)) << "i = " << i;
    }
}

TEST_F(PackerComplexTest, ReadMultipleRecursivelyAddedFiles)
{
    PackerResult pr;

    // initialize mWriter
    EXPECT_NO_THROW(pr = mWriter->Init(TEST_PACK_PATH));
    EXPECT_EQ(PackerResult::OK, pr);

    // add files recursively
    EXPECT_NO_THROW(pr = mWriter->AddFilesRecursively(TEST_SAMPLE_FILE_DIR));
    EXPECT_EQ(PackerResult::OK, pr);

    // write files to PAK archive
    EXPECT_NO_THROW(pr = mWriter->WritePAK());
    ASSERT_EQ(PackerResult::OK, pr);

    // initialize mReader
    EXPECT_NO_THROW(pr = mReader->Init(TEST_PACK_PATH));
    ASSERT_EQ(PackerResult::OK, pr);

    // check file count
    size_t fileCount;
    EXPECT_NO_THROW(fileCount = mReader->GetFileCount());
    ASSERT_EQ(TEST_FILE_COUNT, fileCount);

    std::unique_ptr<Buffer> readData, correctData;
    const std::string prefix = TEST_SAMPLE_FILE_DIR + TEST_SAMPLE_FILE_PREFIX;
    std::string path;

    // read every file and see if data was copied correctly
    for (int i = 0; i < TEST_FILE_COUNT; ++i)
    {
        //initialize loop iteration
        path = prefix + std::to_string(i);
        readData.reset(new Buffer());
        correctData.reset(new Buffer());

        // read file from archive
        EXPECT_NO_THROW(pr = mReader->GetFile(path, *readData)) << "i = " << i;
        ASSERT_EQ(PackerResult::OK, pr) << "i = " << i;

        // read original file
        FILE* pFile = fopen(path.c_str(), "rb");
        ASSERT_NE(nullptr, pFile) << "i = " << i;

        // get file size
        fseek(pFile, 0, SEEK_END);
        uint32 correctDataSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        // create buffer and read data
        correctData->Create(correctDataSize);
        EXPECT_EQ(correctDataSize,
                  fread(correctData->GetData(), sizeof(unsigned char), correctDataSize, pFile));
        fclose(pFile);

        // check data size
        ASSERT_EQ(correctDataSize, readData->GetSize()) << "i = " << i;

        // compare data
        ASSERT_EQ(0, memcmp(readData->GetData(), correctData->GetData(), correctDataSize)) << "i = " << i;
    }
}
