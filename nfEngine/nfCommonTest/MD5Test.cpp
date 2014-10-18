#include "stdafx.hpp"
#include "../nfCommon/MD5.hpp"

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
    // correct hash, taken from references above
    MD5Hash correctHash;
    correctHash.Set(0xd41d8cd9, 0x8f00b204, 0xe9800998, 0xecf8427e);

    EXPECT_NO_THROW(hash.Calculate(""));
    EXPECT_TRUE(hash == correctHash) << "correct: " << correctHash << "\nis:      " << hash;
}

TEST_F(MD5Test, MD5SingleChunkTest)
{
    // correct hash, taken from references above
    MD5Hash correctHash;
    correctHash.Set(0xe2c3e2af, 0xe1985a65, 0x109de214, 0x31f7e8c3);

    EXPECT_NO_THROW(hash.Calculate("this_is_test_file"));
    EXPECT_TRUE(hash == correctHash) << "correct: " << correctHash << "\nis:      " << hash;
}

TEST_F(MD5Test, MD5FullSingleChunkTest)
{
    MD5Hash correctHash;
    correctHash.Set(0xd6b350d7, 0xb9600b35, 0x0d653357, 0x128e10f9);

    EXPECT_NO_THROW(hash.Calculate("here_we_have_a_long_text_which_should_have_56_characters"));
    EXPECT_TRUE(hash == correctHash) << "correct: " << correctHash << "\nis:      " << hash;
}


TEST_F(MD5Test, MD5MultiChunkTest)
{
    MD5Hash correctHash;
    correctHash.Set(0xe5bd67e3, 0xc05fc0a9, 0x3806937c, 0x302d126e);

    EXPECT_NO_THROW(hash.Calculate("and_here_goes_longer_text_which_just_exceeds_56_characters"));
    EXPECT_TRUE(hash == correctHash) << "correct: " << correctHash << "\nis:      " << hash;
}
