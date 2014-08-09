#pragma once

#include "packDLL.h"
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <Windows.h>

#ifdef PACKER_EXPORTS
	#include <zlib.h>
	#include <DynArray.h>
#endif

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

typedef unsigned short int PACK_RESULT;
typedef unsigned __int64 uint64;
typedef __int64 int64;
typedef unsigned __int32 uint32;
typedef __int32 int32;
typedef unsigned __int16 uint16;
typedef __int16 int16;
typedef unsigned char uchar;
typedef std::string str;

#define PACK_OK (0)
#define PACK_ERROR_FILE_NOT_FOUND (1)
#define PACK_ERROR_FILE_NOT_CREATED (2)
#define PACK_ERROR_NULLPOINTER (3)

#define PACK_ERROR_COUT(x) if(x) { cout << "Error " << x << '\n'; return x; }

