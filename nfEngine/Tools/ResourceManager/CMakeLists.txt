# @file
# @author Witek902 (witek902@gmail.com)
# @brief  CMake for ResourceManager tool

MESSAGE("Generating Makefile for ResourceManager")

FILE(GLOB RESOURCE_MANAGER_SOURCES *.cpp)
FILE(GLOB RESOURCE_MANAGER_HEADERS *.hpp)

# Search for dependencies
PKG_CHECK_MODULES(RESOURCE_MANAGER_DEPS REQUIRED
                  xcb) # TODO remove (needed by nfCommon Window)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFENGINE_DIRECTORY}
                    ${RESOURCE_MANAGER_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(ResourceManager ${RESOURCE_MANAGER_SOURCES} ${RESOURCE_MANAGER_HEADERS})
SET_TARGET_PROPERTIES(ResourceManager PROPERTIES
                      LINK_FLAGS "-pthread"
                      COTIRE_CXX_PREFIX_HEADER_INIT "PCH.hpp")

ADD_DEPENDENCIES(ResourceManager nfResources nfCommon)
TARGET_LINK_LIBRARIES(ResourceManager nfResources nfCommon model_obj squish jpeg png dl
                      ${RESOURCE_MANAGER_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET ResourceManager POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ResourceManager> ${NFE_OUTPUT_DIRECTORY}/${targetfile})

cotire(ResourceManager)