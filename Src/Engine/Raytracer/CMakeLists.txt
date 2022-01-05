MESSAGE("Generating Makefile for Raytracer")

SET(NFE_RAYTRACER_DIRECTORY ${NFE_ENGINE_DIRECTORY}/Raytracer)

SET(RAYTRACER_SOURCES
    PCH.cpp
    BVH/BVH.cpp
    BVH/BVHBuilder.cpp
    Color/BlackBodyColor.cpp
    Color/Color.cpp
    Color/ColorRGB.cpp
    Color/MonochromaticColor.cpp
    Color/RayColor.cpp
    Color/Wavelength.cpp
    Material/BSDF/BSDF.cpp
    Material/BSDF/DielectricBSDF.cpp
    Material/BSDF/DiffuseBSDF.cpp
    Material/BSDF/MetalBSDF.cpp
    Material/BSDF/Microfacet.cpp
    Material/BSDF/NullBSDF.cpp
    Material/BSDF/PlasticBSDF.cpp
    Material/BSDF/RoughDielectricBSDF.cpp
    Material/BSDF/RoughDiffuseBSDF.cpp
    Material/BSDF/RoughMetalBSDF.cpp
    Material/BSDF/RoughPlasticBSDF.cpp
    Material/Material.cpp
    Material/MaterialParameter.cpp
    Medium/Medium.cpp
    Medium/PhaseFunction.cpp
    Renderers/DebugRenderer.cpp
    Renderers/LightTracer.cpp
    Renderers/PathTracer.cpp
    Renderers/PathTracerMIS.cpp
    Renderers/Renderer.cpp
    Renderers/RendererContext.cpp
    Renderers/VertexConnectionAndMerging.cpp
    Rendering/Film.cpp
    Rendering/PostProcess.cpp
    Rendering/RenderingContext.cpp
    Rendering/RenderingParams.cpp
    Rendering/Tonemapping.cpp
    Rendering/Viewport.cpp
    Sampling/GenericSampler.cpp
    Sampling/HaltonSampler.cpp
    Scene/Camera.cpp
    Scene/Light/AreaLight.cpp
    Scene/Light/BackgroundLight.cpp
    Scene/Light/DirectionalLight.cpp
    Scene/Light/Light.cpp
    Scene/Light/PointLight.cpp
    Scene/Light/SpotLight.cpp
    Scene/Object/SceneObject.cpp
    Scene/Object/SceneObject_Decal.cpp
    Scene/Object/SceneObject_Light.cpp
    Scene/Object/SceneObject_Shape.cpp
    Scene/Scene.cpp
    Shapes/BoxShape.cpp
    Shapes/CsgShape.cpp
    Shapes/CylinderShape.cpp
    Shapes/Mesh/VertexBuffer.cpp
    Shapes/MeshShape.cpp
    Shapes/RectShape.cpp
    Shapes/Shape.cpp
    Shapes/SphereShape.cpp
    Textures/BitmapTexture.cpp
    Textures/BitmapTexture3D.cpp
    Textures/CheckerboardTexture.cpp
    Textures/ConstTexture.cpp
    Textures/GradientTexture.cpp
    Textures/MixTexture.cpp
    Textures/NoiseTexture.cpp
    Textures/NoiseTexture3D.cpp
    Textures/Texture.cpp
    Traversal/RayPacket.cpp
    Traversal/RayStream.cpp
    Traversal/TraversalContext.cpp
    Traversal/Traversal_Packet.cpp
    Utils/Bitmap.cpp
    Utils/BitmapBMP.cpp
    Utils/BitmapDDS.cpp
    Utils/BitmapEXR.cpp
    Utils/BitmapUtils.cpp
    Utils/BitmapVDB.cpp
    Utils/BlockCompression.cpp
    Utils/KdTree.cpp
    Utils/Memory.cpp
    Utils/Profiler.cpp
    )

SET(RAYTRACER_HEADERS
    PCH.h
    Config.h
    ForwardDeclarations.hpp
    RayLib.h
    Raytracer.h
    BVH/BVH.h
    BVH/BVHBuilder.h
    Color/BlackBodyColor.h
    Color/Color.h
    Color/ColorRGB.h
    Color/MonochromaticColor.h
    Color/RayColor.h
    Color/Wavelength.h
    Material/BSDF/BSDF.h
    Material/BSDF/DielectricBSDF.h
    Material/BSDF/DiffuseBSDF.h
    Material/BSDF/MetalBSDF.h
    Material/BSDF/Microfacet.h
    Material/BSDF/NullBSDF.h
    Material/BSDF/PlasticBSDF.h
    Material/BSDF/RoughDielectricBSDF.h
    Material/BSDF/RoughDiffuseBSDF.h
    Material/BSDF/RoughMetalBSDF.h
    Material/BSDF/RoughPlasticBSDF.h
    Material/Material.h
    Material/MaterialParameter.h
    Medium/Medium.h
    Medium/PhaseFunction.h
    Renderers/DebugRenderer.h
    Renderers/LightTracer.h
    Renderers/PathTracer.h
    Renderers/PathTracerMIS.h
    Renderers/Renderer.h
    Renderers/RendererContext.h
    Renderers/VertexConnectionAndMerging.h
    Rendering/Counters.h
    Rendering/Film.h
    Rendering/PathDebugging.h
    Rendering/PostProcess.h
    Rendering/RenderingContext.h
    Rendering/RenderingParams.h
    Rendering/ShadingData.h
    Rendering/Tonemapping.h
    Rendering/Viewport.h
    Sampling/GenericSampler.h
    Sampling/HaltonSampler.h
    Scene/Camera.h
    Scene/Light/AreaLight.h
    Scene/Light/BackgroundLight.h
    Scene/Light/DirectionalLight.h
    Scene/Light/Light.h
    Scene/Light/PointLight.h
    Scene/Light/SpotLight.h
    Scene/Object/SceneObject.h
    Scene/Object/SceneObject_Decal.h
    Scene/Object/SceneObject_Light.h
    Scene/Object/SceneObject_Shape.h
    Scene/Scene.h
    Shapes/BoxShape.h
    Shapes/CsgShape.h
    Shapes/CylinderShape.h
    Shapes/Mesh/VertexBuffer.h
    Shapes/Mesh/VertexBufferDesc.h
    Shapes/MeshShape.h
    Shapes/RectShape.h
    Shapes/Shape.h
    Shapes/SphereShape.h
    Textures/BitmapTexture.h
    Textures/BitmapTexture3D.h
    Textures/CheckerboardTexture.h
    Textures/ConstTexture.h
    Textures/GradientTexture.h
    Textures/MixTexture.h
    Textures/NoiseTexture.h
    Textures/NoiseTexture3D.h
    Textures/Texture.h
    Traversal/HitPoint.h
    Traversal/Intersection.h
    Traversal/RayPacket.h
    Traversal/RayPacketTypes.h
    Traversal/RayStream.h
    Traversal/TraversalContext.h
    Traversal/Traversal_Packet.h
    Traversal/Traversal_Simd.h
    Traversal/Traversal_Single.h
    Utils/Bitmap.h
    Utils/BitmapUtils.h
    Utils/BlockCompression.h
    Utils/HashGrid.h
    Utils/iacaMarks.h
    Utils/KdTree.h
    Utils/LookupTable.h
    Utils/Memory.h
    Utils/Profiler.h
    )

FILE(GLOB_RECURSE RAYTRACER_EXTERNAL_TINYEXR_SOURCES ${NFEDEPS_ROOT_DIRECTORY}/tinyexr/tinyexr.cc)

ADD_LIBRARY(Raytracer SHARED
            ${RAYTRACER_SOURCES}
            ${RAYTRACER_EXTERNAL_TINYEXR_SOURCES}
            ${RAYTRACER_EXTERNAL_IMGUI_SOURCES}
            )

TARGET_COMPILE_DEFINITIONS(Raytracer
                           PRIVATE NFE_RAYTRACER_EXPORTS
                           )

TARGET_INCLUDE_DIRECTORIES(Raytracer
                           PRIVATE ${NFE_RAYTRACER_DIRECTORY}
                           PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
                           PRIVATE ${NFE_SRC_DIRECTORY}
                           )

TARGET_LINK_DIRECTORIES(Raytracer
                        PRIVATE ${NFEDEPS_LIB_DIRECTORY}
                        PRIVATE ${NFE_OUTPUT_DIRECTORY}
                        )

TARGET_LINK_LIBRARIES(Raytracer Common)

TARGET_PRECOMPILE_HEADERS(Raytracer PRIVATE PCH.h)

ADD_CUSTOM_COMMAND(TARGET Raytracer POST_BUILD COMMAND
                   ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Raytracer> ${NFE_OUTPUT_DIRECTORY}/${targetfile})