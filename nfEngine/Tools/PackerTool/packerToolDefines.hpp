#pragma once

//Exception definition
#include "packerToolException.hpp"

#define SAFE_MEMFREE(x) do { if(x) { delete x; x = 0; } } while(0)
#define SAFE_TABLEFREE(x) do { if(x) { delete[] x; x = 0; } } while(0)
#define SAFE_FILEFREE(x) do { if(x) { fclose(x); x = 0; } } while(0)

extern const std::string configFilePath;
