#include "PCH.hpp"
#include "nfCommon/Utils/MD5.hpp"


using namespace NFE::Common;

class MD5Test : public testing::Test
{
protected:
    MD5Hash hash;
};

/*
 * sample input from:
 *   - http://www.adamek.biz/md5-generator.php
 *   - http://www.md5.cz/
 *   - http://www.miraclesalad.com/webtools/md5.php
 *
 * md5("")                  = "d41d8cd9 8f00b204 e9800998 ecf8427e"
 * md5("this_is_test_file") = "e2c3e2af e1985a65 109de214 31f7e8c3"
 * md5("here_we_have_a_long_text_which_should_have_56_characters")
 *                          = "d6b350d7 b9600b35 0d653357 128e10f9"
 * md5("and_here_goes_longer_text_which_just_exceeds_56_characters")
 *                          = "e5bd67e3 c05fc0a9 3806937c 302d126e"
 */


TEST_F(MD5Test, MD5EmptyTest)
{
    // correct hash, taken from references above, however with correct endianness
    MD5Hash correctHash;
    correctHash.Set(0xd98c1dd4, 0x04b2008f, 0x980980e9, 0x7e42f8ec);

    hash.Calculate("");
    EXPECT_TRUE(hash == correctHash) << "correct: " << correctHash << "\nis:      " << hash;
}

TEST_F(MD5Test, MD5SingleChunkTest)
{
    MD5Hash correctHash;
    correctHash.Set(0xafe2c3e2, 0x655a98e1, 0x14e29d10, 0xc3e8f731);

    hash.Calculate("this_is_test_file");
    EXPECT_TRUE(hash == correctHash) << "correct: " << correctHash << "\nis:      " << hash;
}

TEST_F(MD5Test, MD5FullSingleChunkTest)
{
    MD5Hash correctHash;
    correctHash.Set(0xd750b3d6, 0x350b60b9, 0x5733650d, 0xf9108e12);

    hash.Calculate("here_we_have_a_long_text_which_should_have_56_characters");
    EXPECT_TRUE(hash == correctHash) << "correct: " << correctHash << "\nis:      " << hash;
}


TEST_F(MD5Test, MD5MultiChunkTest)
{
    MD5Hash correctHash;
    correctHash.Set(0xe367bde5, 0xa9c05fc0, 0x7c930638, 0x6e122d30);

    hash.Calculate("and_here_goes_longer_text_which_just_exceeds_56_characters");
    EXPECT_TRUE(hash == correctHash) << "correct: " << correctHash << "\nis:      " << hash;
}
