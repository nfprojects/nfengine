# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for CommonTest

MESSAGE(STATUS "Generating build files for CommonTest")

SET(NFE_COMMON_TEST_DIRECTORY ${NFE_TESTS_DIRECTORY}/CommonTest)

SET(NFCOMMONTEST_SOURCES
    PCH.cpp
    Constants.cpp
    Main.cpp
)

SET(NFCOMMONTEST_HEADERS
    PCH.hpp
    Constants.hpp
    TestCases/Reflection/ReflectionTestCommon.hpp
    TestClasses.hpp
)

SET(NFCOMMONTEST_TEST_CASES_SOURCES
    TestCases/Containers/ArrayViewTest.cpp
    TestCases/Containers/DequeTest.cpp
    TestCases/Containers/DynArrayTest.cpp
    TestCases/Containers/DynArrayTest_Containers.cpp
    TestCases/Containers/FixedArrayTest.cpp
    TestCases/Containers/HashMapTest.cpp
    TestCases/Containers/HashSetTest_Containers.cpp
    TestCases/Containers/MapTest.cpp
    TestCases/Containers/PackedArrayTest.cpp
    TestCases/Containers/SetTest.cpp
    TestCases/Containers/SetTest_Containers.cpp
    TestCases/Containers/SharedPtrTest.cpp
    TestCases/Containers/SharedPtrTest_Containers.cpp
    TestCases/Containers/StaticArrayTest.cpp
    TestCases/Containers/StringTest.cpp
    TestCases/Containers/StringViewTest.cpp
    TestCases/Containers/UniquePtrTest.cpp
    TestCases/Containers/UniquePtrTest_Containers.cpp
    TestCases/Containers/WeakPtrTest.cpp
    TestCases/FileSystem/DirectoryWatchTest.cpp
    TestCases/FileSystem/FileAsyncTest.cpp
    TestCases/FileSystem/FileBufferedTest.cpp
    TestCases/FileSystem/FileSystemTest.cpp
    TestCases/FileSystem/FileTest.cpp
    TestCases/Math/MathBoxTest.cpp
    TestCases/Math/MathConversionsTest.cpp
    TestCases/Math/MathDistributionTest.cpp
    TestCases/Math/MathEquationSolverTest.cpp
    TestCases/Math/MathGeometryTest.cpp
    TestCases/Math/MathMatrix2Test.cpp
    TestCases/Math/MathMatrix3Test.cpp
    TestCases/Math/MathMatrix4Test.cpp
    TestCases/Math/MathPackedTest.cpp
    TestCases/Math/MathQuaternionTest.cpp
    TestCases/Math/MathRayGeometryTest.cpp
    TestCases/Math/MathTest.cpp
    TestCases/Math/MathTranscendental.cpp
    TestCases/Math/MathTransformTest.cpp
    TestCases/Math/MathVec16fTest.cpp
    TestCases/Math/MathVec2fTest.cpp
    TestCases/Math/MathVec3fTest.cpp
    TestCases/Math/MathVec4fTest.cpp
    TestCases/Math/MathVec4fUTest.cpp
    TestCases/Math/MathVec4iTest.cpp
    TestCases/Math/MathVec8fTest.cpp
    TestCases/Math/MathVec8iTest.cpp
    TestCases/Math/RandomTest.cpp
    TestCases/Memory/AlignedTest.cpp
    TestCases/Memory/MemoryTest.cpp
    TestCases/Reflection/ReflectionClassTest.cpp
    TestCases/Reflection/ReflectionDynArrayTest.cpp
    TestCases/Reflection/ReflectionEnumTest.cpp
    TestCases/Reflection/ReflectionFundamentalTypesTest.cpp
    TestCases/Reflection/ReflectionPolymorphicClassTest.cpp
    TestCases/Reflection/ReflectionSerializerTest.cpp
    TestCases/Reflection/ReflectionSharedPtrTest.cpp
    TestCases/Reflection/ReflectionTestCommon.cpp
    TestCases/Reflection/ReflectionTypeMismatchTest.cpp
    TestCases/Reflection/ReflectionUniquePtrTest.cpp
    TestCases/System/ConditionVariableTest.cpp
    TestCases/System/ExclusiveLockTest.cpp
    TestCases/System/SharedLockTest.cpp
    TestCases/System/SystemInfoTest.cpp
    TestCases/System/ThreadTest.cpp
    TestCases/System/TimerTest.cpp
    TestCases/System/WindowTest.cpp
    TestCases/Utils/BitUtilsTest.cpp
    TestCases/Utils/BufferInputStreamTest.cpp
    TestCases/Utils/BufferOutputStreamTest.cpp
    TestCases/Utils/BVHTest.cpp
    TestCases/Utils/CompressedInt.cpp
    TestCases/Utils/ConfigTest.cpp
    TestCases/Utils/EnumIteratorTest.cpp
    TestCases/Utils/ImageTest.cpp
    TestCases/Utils/LatchTest.cpp
    TestCases/Utils/MD5Test.cpp
    TestCases/Utils/StringUtilsTest.cpp
    TestCases/Utils/ThreadPoolSimpleTest.cpp
    TestCases/Utils/ThreadPoolStressTest.cpp
)

IF(UNIX)
    # Search for dependencies
    PKG_CHECK_MODULES(NFCOMMONTEST_DEPS REQUIRED xcb)
ENDIF(UNIX)

ADD_EXECUTABLE(CommonTest ${NFCOMMONTEST_SOURCES} ${NFCOMMONTEST_HEADERS} ${NFCOMMONTEST_TEST_CASES_SOURCES})

IF(UNIX)
    SET_TARGET_PROPERTIES(CommonTest PROPERTIES LINK_FLAGS "-pthread")
ENDIF(UNIX)

TARGET_INCLUDE_DIRECTORIES(CommonTest
    PRIVATE ${NFE_COMMON_TEST_DIRECTORY}
    PRIVATE ${NFE_SRC_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/googletest/googletest/include
    PRIVATE ${NFECOMMONTEST_DEPS_INCLUDE_DIRS}
)

TARGET_LINK_DIRECTORIES(CommonTest
    PRIVATE ${NFEDEPS_LIB_DIRECTORY}
    PRIVATE ${NFE_OUTPUT_DIRECTORY}
)

ADD_DEPENDENCIES(CommonTest Common testCalcLib)

IF(WIN32)
    SET(NFCOMMONTEST_SYSTEM_DEPS )
ELSEIF(UNIX)
    SET(NFCOMMONTEST_SYSTEM_DEPS dl)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(CommonTest Common gtest ${NFCOMMONTEST_SYSTEM_DEPS} ${NFCOMMONTEST_DEPS_LIBRARIES})
TARGET_PRECOMPILE_HEADERS(CommonTest PRIVATE PCH.hpp)

ADD_CUSTOM_COMMAND(TARGET CommonTest POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CommonTest> ${NFE_OUTPUT_DIRECTORY}
)

ADD_SUBDIRECTORY("TestResources")
