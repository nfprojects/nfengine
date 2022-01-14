# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for Core

MESSAGE(STATUS "Generating build files for Core")

SET(NFE_CORE_DIRECTORY ${NFE_ENGINE_DIRECTORY}/Core)

SET(CORE_SOURCES
    PCH.cpp
    Input/InputEvent.cpp
    Scene/Entity.cpp
    Scene/EntityController.cpp
    Scene/Scene.cpp
    Scene/Components/Component.cpp
    Scene/Components/ComponentCamera.cpp
    Scene/Components/ComponentController.cpp
    Scene/Components/ComponentLight.cpp
    Scene/Components/ComponentMesh.cpp
    Scene/Components/ComponentTrigger.cpp
    Scene/Events/Event.cpp
    Scene/Events/Event_Input.cpp
    Scene/Events/Event_Tick.cpp
    Scene/Events/Event_Trigger.cpp
    Scene/Systems/EntitySystem.cpp
    Scene/Systems/EventSystem.cpp
    Scene/Systems/InputSystem.cpp
    Scene/Systems/RendererSystem.cpp
    Scene/Systems/System.cpp
    Scene/Systems/TriggerSystem.cpp
    Utils/ConfigManager.cpp
    Utils/ConfigVariable.cpp
    Utils/Profiler.cpp
)

SET(CORE_HEADERS
    PCH.hpp
    Core.hpp
    ForwardDeclarations.hpp
    ShaderCommon.hpp
    Input/InputEvent.hpp
    Scene/Entity.hpp
    Scene/EntityController.hpp
    Scene/Scene.hpp
    Scene/Components/Component.hpp
    Scene/Components/ComponentCamera.hpp
    Scene/Components/ComponentController.hpp
    Scene/Components/ComponentLight.hpp
    Scene/Components/ComponentMesh.hpp
    Scene/Components/ComponentTrigger.hpp
    Scene/Events/Event.hpp
    Scene/Events/Event_Input.hpp
    Scene/Events/Event_Tick.hpp
    Scene/Events/Event_Trigger.hpp
    Scene/Systems/EntitySystem.hpp
    Scene/Systems/EventSystem.hpp
    Scene/Systems/InputSystem.hpp
    Scene/Systems/RendererSystem.hpp
    Scene/Systems/System.hpp
    Scene/Systems/TriggerSystem.hpp
    Utils/ConfigManager.hpp
    Utils/ConfigVariable.hpp
    Utils/Profiler.hpp
    Utils/SimpleInput.hpp
)

SET(CORE_IMGUI_EXTERNAL_SOURCES
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui.cpp
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui_draw.cpp
    ${NFEDEPS_ROOT_DIRECTORY}/imgui/imgui_widgets.cpp
)

IF(WIN32)
    LIST(APPEND CORE_SOURCES Main.cpp)
ENDIF(WIN32)

ADD_LIBRARY(Core SHARED ${CORE_SOURCES} ${CORE_HEADERS} ${CORE_IMGUI_EXTERNAL_SOURCES})

# TODO rename NF_CORE_EXPORTS -> CORE_EXPORTS or NFE_CORE_EXPORTS
TARGET_COMPILE_DEFINITIONS(Core
    PRIVATE NF_CORE_EXPORTS
)

TARGET_INCLUDE_DIRECTORIES(Core
    PRIVATE ${NFE_CORE_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
    PRIVATE ${NFE_SRC_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/freetype2/include
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/nfenginedeps/rapidxml-1.13
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/imgui
)

TARGET_LINK_DIRECTORIES(Core
    PRIVATE ${NFEDEPS_LIB_DIRECTORY}
    PRIVATE ${NFE_OUTPUT_DIRECTORY}
)

ADD_DEPENDENCIES(Core Common RendererCommon RendererVk)

IF(WIN32)
    ADD_DEPENDENCIES(Core RendererD3D12)
ENDIF(WIN32)

# Core depends on all Renderers, but it doesn't link directly to them.
# Renderer is chosen in runtime.
TARGET_LINK_LIBRARIES(Core Common freetype zlibstatic)

TARGET_PRECOMPILE_HEADERS(Core PRIVATE PCH.hpp)

SET_PROPERTY(TARGET Core PROPERTY FOLDER Src/Engine)
NFE_SOURCE_GROUP_BY_DIR(CORE_SOURCES)
NFE_SOURCE_GROUP_BY_DIR(CORE_HEADERS)
NFE_SOURCE_GROUP_PCH(Core)
SOURCE_GROUP("External/imgui" FILES ${CORE_IMGUI_EXTERNAL_SOURCES})
