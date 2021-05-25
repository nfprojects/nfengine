MESSAGE("Generating Makefile for RaytracerDemo")

SET(NFE_RAYTRACER_DEMO_DIRECTORY ${NFE_APPS_DIRECTORY}/RaytracerDemo)

FILE(GLOB RAYTRACER_DEMO_SOURCES *.cpp)
FILE(GLOB RAYTRACER_DEMO_LINUX_SOURCES Linux/*.cpp)
FILE(GLOB RAYTRACER_DEMO_HEADERS *.h)

SET(RAYTRACER_EXTERNAL_IMGUI_SOURCES
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui.cpp
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui_draw.cpp
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui_widgets.cpp
    ${NFEDEPS_ROOT_DIRECTORY}/imgui_sw/src/imgui_sw.cpp
)
SET(RAYTRACER_EXTERNAL_TINYOBJ_SOURCES
    ${NFEDEPS_ROOT_DIRECTORY}/tinyobjloader/tiny_obj_loader.cc
)

ADD_EXECUTABLE(RaytracerDemo
    ${RAYTRACER_DEMO_SOURCES}
    ${RAYTRACER_EXTERNAL_IMGUI_SOURCES}
    ${RAYTRACER_EXTERNAL_TINYOBJ_SOURCES}
    ${RAYTRACER_DEMO_LINUX_SOURCES}
)

TARGET_INCLUDE_DIRECTORIES(RaytracerDemo
                           PRIVATE ${NFE_RAYTRACER_DEMO_DIRECTORY}
                           PRIVATE ${NFE_SRC_DIRECTORY}
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/rapidjson/include/
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/cxxopts/include/
                           PRIVATE ${RT_ROOT_DIRECTORY}/External/
)

TARGET_LINK_DIRECTORIES(RaytracerDemo
                        PRIVATE ${NFEDEPS_LIB_DIRECTORY}
                        PRIVATE ${NFE_OUTPUT_DIRECTORY}
)

SET_TARGET_PROPERTIES(RaytracerDemo PROPERTIES LINK_FLAGS "-pthread")

ADD_DEPENDENCIES(RaytracerDemo Raytracer RendererCommon)
TARGET_LINK_LIBRARIES(RaytracerDemo Raytracer RendererCommon)
TARGET_PRECOMPILE_HEADERS(RaytracerDemo PRIVATE PCH.h)

ADD_CUSTOM_COMMAND(TARGET RaytracerDemo POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RaytracerDemo> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
