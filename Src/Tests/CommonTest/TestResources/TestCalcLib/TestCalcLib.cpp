/**
 * @file
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief Definition of test library for LibraryTest
 */

#include "TestCalcLib.hpp"


extern "C" int Add(int num1, int num2)
{
    return num1 + num2;
}

extern "C" int Subtract(int num1, int num2)
{
    return num1 - num2;
}
