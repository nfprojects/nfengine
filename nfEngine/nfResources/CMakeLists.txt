# @file
# @author Witek902 (witek902@gmail.com)
# @brief  CMake for nfResources

MESSAGE("Generating Makefile for nfResources")

FILE(GLOB NFRESOURCES_SOURCES *.cpp)
FILE(GLOB NFRESOURCES_HEADERS *.hpp)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFENGINE_DIRECTORY}/)
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_LIBRARY(nfResources STATIC ${NFRESOURCES_SOURCES} ${NFRESOURCES_HEADERS})
SET_TARGET_PROPERTIES(nfResources PROPERTIES
                      COMPILE_FLAGS "-pthread"
                      LINK_FLAGS "-pthread")

ADD_DEPENDENCIES(nfResources nfCommon)
TARGET_LINK_LIBRARIES(nfResources nfCommon squish jpeg png dl ${NFRESOURCES_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET nfResources POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfResources> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
