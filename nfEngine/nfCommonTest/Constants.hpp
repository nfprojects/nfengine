#pragma once

#include <string>

// libpacker-related constants
extern const std::string TEST_PACK_PATH;
extern const std::string TEST_SAMPLE_FILE_DIR;
extern const std::string TEST_SAMPLE_FILE_PREFIX;

// BufferOutputStream and BufferInputStream constants
extern const std::string TEXT;
extern const size_t TEXTSIZE;

// Image constants
extern const std::string testImagesPath;
extern const int textureWidth;
extern const int textureHeight;
extern const std::string textureDDS_BC1;
extern const std::string textureDDS_BC2;
extern const std::string textureDDS_BC3;
extern const std::string textureDDS_BC4;
extern const std::string textureDDS_BC5;
extern const std::string textureDDS_BC6H;
extern const std::string textureDDS_BC7;
extern const std::string textureDDS_MM;
extern const std::string textureJPG;
extern const std::string textureBMP16ARGB;
extern const std::string textureBMP16XRGB;
extern const std::string textureBMP16RGB;
extern const std::string textureBMP24;
extern const std::string textureBMP32ARGB;
extern const std::string textureBMP32XRGB;
extern const std::string texturePNG_RGB;
extern const std::string texturePNG_RGBA;
extern const std::string texturePNG_A;
extern const int brbgTestSquareWidth;
extern const int brbgTestSquareHeight;
extern const int brbgTestSquareDataSize; // = brbgTestSquareWidth * brbgTestSquareHeight * 3;
extern unsigned char brbgTestSquare[48]; //[brbgTestSquareDataSize];
extern const int maxTestSquareMipmapNum;