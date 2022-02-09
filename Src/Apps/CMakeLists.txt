SET(NFE_APPS_DIRECTORY ${NFE_SRC_DIRECTORY}/Apps)

ADD_SUBDIRECTORY("RaytracerDemo")
ADD_SUBDIRECTORY("RendererDemo")

# EngineDemo needs rewrite for other platforms - uses WinMain and other Windows-only parts
IF(WIN32)
    ADD_SUBDIRECTORY("EngineDemo")
    SET(NFE_ENGINEDEMO_DEPENDENCY EngineDemo)
ELSEIF(UNIX)
    SET(NFE_ENGINEDEMO_DEPENDENCY )
ELSE(WIN32)
    MESSAGE(FATAL_ERROR "Target platform not supported")
ENDIF(WIN32)

# Meta target to build all test apps
ADD_CUSTOM_TARGET(Apps_All DEPENDS RendererDemo RaytracerDemo ${NFE_ENGINEDEMO_DEPENDENCY}
    COMMENT "Build all apps and their dependencies"
)
