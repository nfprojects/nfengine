#pragma once

#include <string>

// libpacker-related constants
const std::string TEST_PACK_PATH = "testfile.nfp";
const std::string TEST_SAMPLE_FILE_DIR = "./TestDir/";
const std::string TEST_SAMPLE_FILE_PREFIX = "samplefile_";

// BufferOutputStream and BufferInputStream constants
// TEXT const used in ImageTest as well
const std::string TEXT = "The quick brown fox jumps over the lazy dog";
const size_t TEXTSIZE = TEXT.length();

// Image constants
const std::string textureDDS_BC1 = "../TestResources/ImageSamples/textureBC1.dds";
const std::string textureDDS_BC2 = "../TestResources/ImageSamples/textureBC2.dds";
const std::string textureDDS_BC3 = "../TestResources/ImageSamples/textureBC3.dds";
const std::string textureDDS_BC4 = "../TestResources/ImageSamples/textureBC4.dds";
const std::string textureDDS_BC5 = "../TestResources/ImageSamples/textureBC5.dds";
const std::string textureJPG = "../TestResources/ImageSamples/textureJPG.jpg";
const std::string textureBMP = "../TestResources/ImageSamples/textureBMP.bmp";
const std::string texturePNG = "../TestResources/ImageSamples/texturePNG.png";