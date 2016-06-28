# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for nfRendererDemo

MESSAGE("Generating Makefile for nfRendererDemo")

FILE(GLOB NFRENDERERDEMO_SOURCES *.cpp)
FILE(GLOB NFRENDERERDEMO_HEADERS *.hpp)
FILE(GLOB NFRENDERERDEMO_SCENE_SOURCES Scenes/*.cpp)
FILE(GLOB NFRENDERERDEMO_SCENE_HEADERS Scenes/*.hpp)

# Search for dependencies
PKG_CHECK_MODULES(NFRENDERERDEMO_DEPS REQUIRED
                  x11
                  gl)

# setup directories
INCLUDE_DIRECTORIES(${NFE_ROOT_DIRECTORY}/nfEngineDeps/
                    ${NFE_ROOT_DIRECTORY}/nfEngine/
                    ${NFRENDERERDEPS_DEPS_INCLUDE_DIRS})
LINK_DIRECTORIES(${NFEDEPS_LIB_DIRECTORY}
                 ${NFE_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(nfRendererDemo ${NFRENDERERDEMO_SOURCES} ${NFRENDERERDEMO_HEADERS}
                              ${NFRENDERERDEMO_SCENE_SOURCES} ${NFRENDERERDEMO_SCENE_HEADERS})
SET_TARGET_PROPERTIES(nfRendererDemo PROPERTIES
                      COMPILE_FLAGS "-pthread -lrt"
                      LINK_FLAGS "-pthread -lrt")

# nfRendererOGL4 dependency removed temporarily
ADD_DEPENDENCIES(nfRendererDemo nfCommon)
TARGET_LINK_LIBRARIES(nfRendererDemo nfCommon squish jpeg png
                      ${NFRENDERERDEMO_DEPS_LIBRARIES})
ADD_CUSTOM_COMMAND(TARGET nfRendererDemo POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:nfRendererDemo>
                                                    ${NFE_OUTPUT_DIRECTORY}/${targetfile})
