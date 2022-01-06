SET(NFE_TESTS_DIRECTORY ${NFE_SRC_DIRECTORY}/Tests)

ADD_SUBDIRECTORY("CommonPerfTest")
ADD_SUBDIRECTORY("CommonTest")
#ADD_SUBDIRECTORY("RaytracerTests")
ADD_SUBDIRECTORY("RendererTest")

# Meta target to build all test apps
ADD_CUSTOM_TARGET(Tests_All DEPENDS CommonTest CommonPerfTest RendererTest
    COMMENT "Build all tests and their dependencies"
)
