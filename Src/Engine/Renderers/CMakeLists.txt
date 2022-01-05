# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for Renderers

SET(NFE_RENDERERS_DIRECTORY ${NFE_ENGINE_DIRECTORY}/Renderers)

ADD_SUBDIRECTORY("RendererCommon")

IF(WIN32)
    ADD_SUBDIRECTORY("RendererD3D12")
ENDIF(WIN32)

ADD_SUBDIRECTORY("RendererVk")
