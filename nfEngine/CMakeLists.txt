# @file
# @author LKostyra (costyrra.xl@gmail.com)
# @brief  CMake for nfEngine

ADD_SUBDIRECTORY("nfCommon")
ADD_SUBDIRECTORY("nfCommonTest")
ADD_SUBDIRECTORY("TestResources")
ADD_SUBDIRECTORY("Renderers")
ADD_SUBDIRECTORY("nfRendererDemo")

# TODO uncomment during Linux version implementation
#ADD_SUBDIRECTORY("nfCore")
#ADD_SUBDIRECTORY("nfCoreTest")
#ADD_SUBDIRECTORY("Tools")
