#pragma once

#include "../Common/nfCommon.hpp"
#include "ForwardDeclarations.hpp"


 // DLL import / export macro
#ifdef NFE_RENDERER_EXPORTS
#define NFE_RENDERER_API __declspec(dllexport)
#else
#define NFE_RENDERER_API __declspec(dllimport)
#endif