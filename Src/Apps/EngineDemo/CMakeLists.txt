# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for EngineDemo

MESSAGE(STATUS "Generating build files for EngineDemo")

SET(NFE_ENGINE_DEMO_DIRECTORY ${NFE_APPS_DIRECTORY}/EngineDemo)

SET(NFENGINEDEMO_SOURCES
    PCH.cpp
    Main.cpp
    GameWindow.cpp
    Controllers/FlickeringLightController.cpp
    Controllers/FreeCameraController.cpp
    Controllers/TriggeredLightController.cpp
)

SET(NFENGINEDEMO_HEADERS
    PCH.hpp
    Main.hpp
    GameWindow.hpp
    Controllers/FlickeringLightController.hpp
    Controllers/FreeCameraController.hpp
    Controllers/TriggeredLightController.hpp
)

ADD_EXECUTABLE(EngineDemo ${NFENGINEDEMO_SOURCES} ${NFENGINEDEMO_HEADERS})

TARGET_INCLUDE_DIRECTORIES(EngineDemo
    PRIVATE ${NFE_ENGINE_DEMO_DIRECTORY}
    PRIVATE ${NFE_SRC_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
)

TARGET_LINK_DIRECTORIES(EngineDemo
    PRIVATE ${NFEDEPS_LIB_DIRECTORY}
    PRIVATE ${NFE_OUTPUT_DIRECTORY}
)

IF(UNIX)
    SET_TARGET_PROPERTIES(EngineDemo PROPERTIES LINK_FLAGS "-pthread")
ENDIF(UNIX)

TARGET_PRECOMPILE_HEADERS(EngineDemo PRIVATE PCH.hpp)

ADD_DEPENDENCIES(EngineDemo Common Core)

TARGET_LINK_LIBRARIES(EngineDemo Common Core)

SET_PROPERTY(TARGET EngineDemo PROPERTY FOLDER Src/Apps)
NFE_SOURCE_GROUP_BY_DIR(NFENGINEDEMO_SOURCES)
NFE_SOURCE_GROUP_BY_DIR(NFENGINEDEMO_HEADERS)
NFE_SOURCE_GROUP_PCH(EngineDemo)
