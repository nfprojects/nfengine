# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for nfEngine

SET(NFE_ENGINE_DIRECTORY ${NFE_SRC_DIRECTORY}/Engine)

ADD_SUBDIRECTORY("Common")
ADD_SUBDIRECTORY("Raytracer")
ADD_SUBDIRECTORY("Renderers")
# Core is temporarily excluded from build
#ADD_SUBDIRECTORY("Core")
