# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake module defining source group related utility functions


# @f NFE_SOURCE_GROUP_BY_DIR
#
# Assigns a source group to each file from file list according to its directory hierarchy.
# Helps to reflect project's dir structure inside Visual Studio as filters.
FUNCTION(NFE_SOURCE_GROUP_BY_DIR FILE_LIST)
    MESSAGE(VERBOSE "Assigning source group per file's directory:")
    FOREACH(F IN LISTS ${FILE_LIST})
        GET_FILENAME_COMPONENT(D ${F} DIRECTORY)
        IF(NOT "${D}" STREQUAL "")
            MESSAGE(VERBOSE "  -> ${F} => ${D}")
            SOURCE_GROUP(${D} FILES ${F})
        ELSE(NOT "${D}" STREQUAL "")
            MESSAGE(VERBOSE "  -> ${F} => root")
            SOURCE_GROUP("" FILES ${F})
        ENDIF(NOT "${D}" STREQUAL "")
    ENDFOREACH(F)
ENDFUNCTION()


# @f NFE_SOURCE_GROUP_PCH
#
# Moves Precompiled Header files to predefined PCH filter in Visual Studio
FUNCTION(NFE_SOURCE_GROUP_PCH TARGET)
    SET(PCH_GROUP_NAME "CMake PCH")
    SOURCE_GROUP(${PCH_GROUP_NAME} FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/cmake_pch.cxx)
    SOURCE_GROUP(${PCH_GROUP_NAME} FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/Debug/cmake_pch.hxx)
    SOURCE_GROUP(${PCH_GROUP_NAME} FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/MinSizeRel/cmake_pch.hxx)
    SOURCE_GROUP(${PCH_GROUP_NAME} FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/Release/cmake_pch.hxx)
    SOURCE_GROUP(${PCH_GROUP_NAME} FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/RelWithDebInfo/cmake_pch.hxx)
ENDFUNCTION()
