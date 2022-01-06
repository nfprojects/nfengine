# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for nfEngine

SET(NFE_ENGINE_DIRECTORY ${NFE_SRC_DIRECTORY}/Engine)

ADD_SUBDIRECTORY("Common")
ADD_SUBDIRECTORY("Raytracer")
ADD_SUBDIRECTORY("Renderers")
# Core is temporarily excluded from build
#ADD_SUBDIRECTORY("Core")

IF(WIN32)
    SET(ENGINE_RENDERERD3D12_DEPENDENCY RendererD3D12)
ELSE(WIN32)
    SET(ENGINE_RENDERERD3D12_DEPENDENCY )
ENDIF(WIN32)

ADD_CUSTOM_TARGET(Engine_All DEPENDS Common Raytracer RendererCommon RendererVk ${ENGINE_RENDERERD3D12_DEPENDENCY}
    COMMENT "Build all Engine projects and dependencies"
)
