SET(NFE_APPS_DIRECTORY ${NFE_SRC_DIRECTORY}/Apps)

# ADD_SUBDIRECTORY("EngineDemo")
ADD_SUBDIRECTORY("RaytracerDemo")
ADD_SUBDIRECTORY("RendererDemo")

# Meta target to build all test apps
ADD_CUSTOM_TARGET(Apps_All DEPENDS RaytracerDemo RendererDemo
    COMMENT "Build all apps and their dependencies"
)
