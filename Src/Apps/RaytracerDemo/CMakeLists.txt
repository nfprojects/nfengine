MESSAGE("Generating Makefile for RaytracerDemo")

SET(NFE_RAYTRACER_DEMO_DIRECTORY ${NFE_APPS_DIRECTORY}/RaytracerDemo)

SET(RAYTRACER_DEMO_SOURCES
    PCH.cpp
    CustomScene.cpp
    Demo.cpp
    DemoRenderer.cpp
    Demo_UserInterface.cpp
    Main.cpp
    MeshLoader.cpp
    ObjectEditor.cpp
    SceneLoader.cpp
)

SET(RAYTRACER_DEMO_HEADERS
    PCH.h
    Demo.h
    DemoRenderer.h
    MeshLoader.h
    ObjectEditor.h
    SceneLoader.h
)

SET(RAYTRACER_EXTERNAL_IMGUI_SOURCES
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui.cpp
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui_draw.cpp
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui_widgets.cpp
)

SET(RAYTRACER_EXTERNAL_TINYOBJ_SOURCES
    ${NFEDEPS_ROOT_DIRECTORY}/tinyobjloader/tiny_obj_loader.cc
)

ADD_EXECUTABLE(RaytracerDemo
    ${RAYTRACER_DEMO_SOURCES}
    ${RAYTRACER_EXTERNAL_IMGUI_SOURCES}
    ${RAYTRACER_EXTERNAL_TINYOBJ_SOURCES}
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

IF(UNIX)
    SET_TARGET_PROPERTIES(RaytracerDemo PROPERTIES LINK_FLAGS "-pthread")
ENDIF(UNIX)

ADD_DEPENDENCIES(RaytracerDemo Raytracer RendererCommon)
TARGET_LINK_LIBRARIES(RaytracerDemo Raytracer RendererCommon)
TARGET_PRECOMPILE_HEADERS(RaytracerDemo PRIVATE PCH.h)

ADD_CUSTOM_COMMAND(TARGET RaytracerDemo POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RaytracerDemo> ${NFE_OUTPUT_DIRECTORY}/${targetfile})
