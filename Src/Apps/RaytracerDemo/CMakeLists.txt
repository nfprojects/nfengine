MESSAGE(STATUS "Generating build files for RaytracerDemo")

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

ADD_EXECUTABLE(RaytracerDemo
    ${RAYTRACER_DEMO_SOURCES}
    ${RAYTRACER_DEMO_HEADERS}
    ${RAYTRACER_EXTERNAL_IMGUI_SOURCES}
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

ADD_DEPENDENCIES(RaytracerDemo Raytracer RendererCommon tinyobjloader cxxopts RendererVk)
IF(WIN32)
    ADD_DEPENDENCIES(RaytracerDemo RendererD3D12)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(RaytracerDemo Raytracer RendererCommon tinyobjloader)
TARGET_PRECOMPILE_HEADERS(RaytracerDemo PRIVATE PCH.h)

SET_PROPERTY(TARGET RaytracerDemo PROPERTY FOLDER Src/Apps)
NFE_SOURCE_GROUP_BY_DIR(RAYTRACER_DEMO_SOURCES)
NFE_SOURCE_GROUP_BY_DIR(RAYTRACER_DEMO_HEADERS)
NFE_SOURCE_GROUP_PCH(RaytracerDemo)
SOURCE_GROUP("External/imgui" FILES ${RAYTRACER_EXTERNAL_IMGUI_SOURCES})
