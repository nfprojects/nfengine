#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#include <windows.h>


//DirectX API
#include <DXGI.h>
#include <D3Dcompiler.h>
#include <D3D11.h>
//#include <dinput.h>

// STL
#include <vector>
#include <map>
#include <string>

// C lib
#include <sys/stat.h>


// TODO: just temporary, fonts management should be done in engine / nfCommon
//FreeType library
#include "ft2build.h"
#include "freetype\freetype.h"
#include "freetype\ftglyph.h"
#include "freetype\ftoutln.h"
#include "freetype\fttrigon.h"
