// simple dynamic library for testing Library module

#ifdef TESTCALCDLL_EXPORTS
#define TESTCALCDLL_API __declspec(dllexport) 
#else
#define TESTCALCDLL_API __declspec(dllimport) 
#endif

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

extern "C" TESTCALCDLL_API int Add(int num1, int num2);
extern "C" TESTCALCDLL_API int Subtract(int num1, int num2);