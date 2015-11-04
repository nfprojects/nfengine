#pragma once

//ignore pointless warnings
#pragma warning (disable: 4251)
#pragma warning (disable: 4100)

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <cstdio>
#include <cstdlib>

// TODO: remove this dependencies. Engine should include everything what is needed
#include <string>
#include <map>
#include <vector>
#include <set>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
