# @file
# @author Witek902 (witek902@gmail.com)
# @brief  CMake for ResourceManager tool

MESSAGE("Generating Makefile for ResourceManager")

FILE(GLOB RESOURCE_MANAGER_SOURCES *.cpp)
FILE(GLOB RESOURCE_MANAGER_HEADERS *.hpp)

# Search for dependencies
PKG_CHECK_MODULES(RESOURCE_MANAGER_DEPS REQUIRED
                  x11
                  gl) # TODO remove (needed by nfCommon Window)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFCOMMON_DIRECTORY}
                    ${RESOURCE_MANAGER_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(ResourceManager ${RESOURCE_MANAGER_SOURCES} ${RESOURCE_MANAGER_HEADERS})
SET_TARGET_PROPERTIES(ResourceManager PROPERTIES
                      COMPILE_FLAGS "-pthread"
                      LINK_FLAGS "-pthread")
ADD_DEPENDENCIES(ResourceManager nfCommon nfResources)
TARGET_LINK_LIBRARIES(ResourceManager nfCommon model_obj squish jpeg png ${RESOURCE_MANAGER_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET ResourceManager POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ResourceManager> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
