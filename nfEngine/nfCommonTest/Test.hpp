#pragma once

#include <string>

// libpacker-related constants
const std::string TEST_PACK_PATH = "testfile.nfp";
const std::string TEST_SAMPLE_FILE_DIR = "./TestDir/";
const std::string TEST_SAMPLE_FILE_PREFIX = "samplefile_";

// BufferOutputStream and BufferInputStream constants
const std::string TEXT = "The quick brown fox jumps over the lazy dog";
const size_t TEXTSIZE = TEXT.length();

// Image constants
const std::string testImagesPath = "../../../nfEngine/TestResources/ImageSamples/";
const std::string textureDDS_BC1 = "textureBC1.dds";
const std::string textureDDS_BC2 = "textureBC2.dds";
const std::string textureDDS_BC3 = "textureBC3.dds";
const std::string textureDDS_BC4 = "textureBC4.dds";
const std::string textureDDS_BC5 = "textureBC5.dds";
const std::string textureDDS_MM = "textureBC1_MM.dds";
const std::string textureJPG = "textureJPG.jpg";
const std::string textureBMP = "textureBMP1.bmp";
const std::string texturePNG_RGB = "texturePNG_RGB.png";
const std::string texturePNG_RGBA = "texturePNG_RGBA.png";
const std::string texturePNG_A = "texturePNG_A.png";