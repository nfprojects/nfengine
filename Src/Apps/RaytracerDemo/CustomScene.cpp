#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "Engine/Raytracer/Textures/NoiseTexture.h"
#include "Engine/Raytracer/Textures/NoiseTexture3D.h"
#include "Engine/Raytracer/Textures/CheckerboardTexture.h"
#include "Engine/Raytracer/Textures/GradientTexture.h"
#include "Engine/Raytracer/Textures/MixTexture.h"
#include "Engine/Raytracer/Textures/BitmapTexture3D.h"

#include "Engine/Raytracer/Scene/Light/DirectionalLight.h"
#include "Engine/Raytracer/Scene/Light/BackgroundLight.h"
#include "Engine/Raytracer/Scene/Light/PointLight.h"
#include "Engine/Raytracer/Scene/Light/AreaLight.h"

#include "Engine/Raytracer/Scene/Object/SceneObject_Shape.h"
#include "Engine/Raytracer/Scene/Object/SceneObject_Light.h"
#include "Engine/Raytracer/Scene/Object/SceneObject_Decal.h"

#include "Engine/Raytracer/Shapes/RectShape.h"
#include "Engine/Raytracer/Shapes/BoxShape.h"
#include "Engine/Raytracer/Shapes/SphereShape.h"

#include "Engine/Raytracer/Medium/Medium.h"
#include "Engine/Raytracer/Medium/PhaseFunction.h"

namespace NFE {
namespace helpers {

using namespace RT;
using namespace Math;
using namespace Common;

bool LoadCustomScene(Scene& scene, Camera& camera)
{
    auto bitmapTextureA = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/default.bmp");
    auto backgroundTexture = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/ENV/Topanga_R32G32B32_FLOAT.dds");
    //auto bitmapTextureB = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/Portal/dirty4x4.bmp");

    Random random;

    // floor
    {
        auto material = Material::Create();
        material->debugName = "floor";
        material->SetBsdf(String("diffuse"));
        material->baseColor = Math::HdrColorRGB(0.9f, 0.9f, 0.9f);
        material->roughness = 0.2f;
        material->Compile();
        material->baseColor.SetTexture(bitmapTextureA);

        const Vec2f size(5000.0f, 5000.0f);
        const Vec2f texScale(0.5f, 0.5f);
        auto rect = MakeSharedPtr<RectShape>(size, texScale);
        UniquePtr<ShapeSceneObject> instance = MakeUniquePtr<ShapeSceneObject>(std::move(rect));
        instance->BindMaterial(material);
        instance->SetTransform(Quaternion::FromEulerAngles(Vec3f(-NFE_MATH_PI / 2.0f, -0.01f, 0.0f)).ToMatrix());
        scene.AddObject(std::move(instance));
    }

    /*
    // many lights test
    {
        for (int32 i = 0; i < 40000; ++i)
        {
            auto material = Material::Create();

            if (i % 3 == 0)
            {
                material->debugName = "default";
                material->SetBsdf("dielectric");
                material->baseColor = Math::HdrColorRGB(Vec4f(1.0f, 1.0f, 1.0f));
                material->Compile();
            }
            else
            {
                material->debugName = "default";
                material->SetBsdf("roughPlastic");
                material->baseColor = Math::HdrColorRGB(Vec4f::Sqrt(random.GetVec4f()));
                material->roughness = 0.1f + random.GetFloat() * 0.4f;
                material->Compile();
            }

            const float r = 0.5f + 5.0f * random.GetFloat();
            const Vec2f posXZ = (random.GetVec2f() - Vec2f(0.5f, 0.5f)) * 4000.0f;
            const Matrix4 translationMatrix = Matrix4::MakeTranslation(Vec4f(posXZ.x, r, posXZ.y));

            auto sphere = MakeUniquePtr<SphereShape>(r);
            UniquePtr<ShapeSceneObject> instance = MakeUniquePtr<ShapeSceneObject>(std::move(sphere));
            instance->BindMaterial(material);
            instance->SetTransform(translationMatrix);
            scene.AddObject(std::move(instance));
        }

        for (int32 i = 0; i < 2000; ++i)
        {
            const float r = 1.0f + 5.0f * random.GetFloat();
            const Vec2f posXZ = (random.GetVec2f() - Vec2f(0.5f, 0.5f)) * 4000.0f;
            const Matrix4 translationMatrix = Matrix4::MakeTranslation(Vec4f(posXZ.x, 10.0f + r, posXZ.y));

            const HdrColorRGB lightColor(Vec4f(10.0f, 10.0f, 10.0f) + random.GetVec4f() * 10.0f);
            auto sphere = MakeUniquePtr<SphereShape>(r);
            auto light = MakeUniquePtr<AreaLight>(std::move(sphere), HdrColorRGB(lightColor));
            auto lightObject = MakeUniquePtr<LightSceneObject>(std::move(light));
            lightObject->SetTransform(translationMatrix);
            scene.AddObject(std::move(lightObject));
        }
    }
    */

    // emissive box
    /*
    {
        auto material = Material::Create();
        material->debugName = "default";
        material->SetBsdf("diffuse");
        material->baseColor.baseValue = HdrColorRGB();
        material->emission.baseValue = HdrColorRGB(1.0f, 1.0f, 1.0f);
        material->Compile();

        ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(0.2f, 5.0f, 5.0f));
        ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        instance->BindMaterial(material);
        instance->SetTransform(Matrix4::MakeTranslation(Vec4f(10.0f, 5.0, 0.0f)));
        scene.AddObject(std::move(instance));
    }
    */

    // volumetric rendering test
    {
        // homogenous emissive medium (box)
        {
            ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f, 1.0f, 1.0f));
            TexturePtr noiseTexture = MakeUniquePtr<NoiseTexture3D>();
            MediumPtr medium = MakeUniquePtr<HeterogeneousEmissiveMedium>(noiseTexture, Vec4f(0.5f, 1.0, 2.0f));

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(4.0f, 1.001f, -4.0f)));
            scene.AddObject(std::move(object));
        }
        // heterogeneous emissive medium (box)
        {
            ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f, 1.0f, 1.0f));
            MediumPtr medium = MakeUniquePtr<HomogenousEmissiveMedium>(Vec4f(0.5f, 1.0, 2.0f));

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(4.0f, 1.001f, 0.0f)));
            scene.AddObject(std::move(object));
        }
        // homogenous emissive sphere (sphere)
        {
            ShapePtr shape = MakeUniquePtr<SphereShape>(1.0f);
            MediumPtr medium = MakeUniquePtr<HomogenousEmissiveMedium>(Vec4f(0.5f, 1.0, 2.0f));

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(4.0f, 1.001f, 4.0f)));
            scene.AddObject(std::move(object));
        }



        // homogenous scattering medium (box)
        {
            const float density = 10.0f;
            const Vec4f scattering = Vec4f(0.25f, 0.5f, 1.0f) * density;
            const Vec4f attenuation = Vec4f(0.0f, 0.0f, 0.0f) * density;
            const Vec4f extintion = attenuation + scattering;
            const Vec4f scatteringAlbedo = scattering / extintion;

            ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f, 1.0f, 1.0f));
            MediumPtr medium = MakeUniquePtr<HomogenousScatteringMedium>(extintion, scatteringAlbedo);

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(0.0f, 1.001f, 0.0f)));
            scene.AddObject(std::move(object));
        }
        // heterogeneous scattering medium (box)
        {
            const float density = 10.0f;
            const Vec4f scattering = Vec4f(0.25f, 0.5f, 1.0f) * density;
            const Vec4f attenuation = Vec4f(0.0f, 0.0f, 0.0f) * density;
            const Vec4f extintion = attenuation + scattering;
            const Vec4f scatteringAlbedo = scattering / extintion;

            ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f, 1.0f, 1.0f));
            TexturePtr noiseTexture = MakeUniquePtr<NoiseTexture3D>();
            MediumPtr medium = MakeUniquePtr<HeterogeneousScatteringMedium>(noiseTexture, extintion, scatteringAlbedo);

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(0.0f, 1.001f, -4.0f)));
            scene.AddObject(std::move(object));
        }
        // homogenous scattering medium (sphere)
        {
            const float density = 10.0f;
            const Vec4f scattering = Vec4f(0.25f, 0.5f, 1.0f) * density;
            const Vec4f attenuation = Vec4f(0.0f, 0.0f, 0.0f) * density;
            const Vec4f extintion = attenuation + scattering;
            const Vec4f scatteringAlbedo = scattering / extintion;

            ShapePtr shape = MakeUniquePtr<SphereShape>(1.0f);
            MediumPtr medium = MakeUniquePtr<HomogenousScatteringMedium>(extintion, scatteringAlbedo);

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(0.0f, 1.001f, 4.0f)));
            scene.AddObject(std::move(object));
        }


        // homogenous absorption medium (box)
        {
            const Vec4f attenuation = Vec4f(0.5f, 1.0f, 2.0f);

            ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f, 1.0f, 1.0f));
            MediumPtr medium = MakeUniquePtr<HomogenousAbsorptiveMedium>(attenuation);

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(-4.0f, 1.001f, 0.0f)));
            scene.AddObject(std::move(object));
        }
        // heterogeneous absorption medium (box)
        {
            const Vec4f attenuation = Vec4f(0.5f, 1.0f, 2.0f);

            ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f, 1.0f, 1.0f));
            TexturePtr noiseTexture = MakeUniquePtr<NoiseTexture3D>();
            MediumPtr medium = MakeUniquePtr<HeterogeneousAbsorptiveMedium>(noiseTexture, attenuation);

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(-4.0f, 1.001f, -4.0f)));
            scene.AddObject(std::move(object));
        }
        // homogenous absorption sphere (sphere)
        {
            const Vec4f attenuation = Vec4f(0.5f, 1.0f, 2.0f);

            ShapePtr shape = MakeUniquePtr<SphereShape>(1.0f);
            MediumPtr medium = MakeUniquePtr<HomogenousAbsorptiveMedium>(attenuation);

            auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
            object->BindMedium(medium);
            object->SetTransform(Matrix4::MakeTranslation(Vec4f(-4.0f, 1.001f, 4.0f)));
            scene.AddObject(std::move(object));
        }
    }

    /*
    // homogenous scattering medium
    {
        const float density = 4.0f;
        const Vec4f scattering = Vec4f(0.25f, 0.5f, 1.0f) * density;
        const Vec4f attenuation = Vec4f(0.0f, 0.0f, 0.0f) * density;

        const Vec4f extintion = attenuation + scattering;
        const Vec4f scatteringAlbedo = scattering / extintion;

        ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousScatteringMedium>(extintion, scatteringAlbedo);

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vec4f(0.0f, 5.001f, 15.0f)));
        scene.AddObject(std::move(object));
    }

    // homogenous absorption medium
    {
        const float density = 1.0f;
        const Vec4f attenuation = Vec4f(1.0f, 0.5f, 0.25f) * density;

        ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousAbsorptiveMedium>(attenuation);

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vec4f(12.0f, 5.001f, 15.0f)));
        scene.AddObject(std::move(object));
    }

    // homogenous emissive medium
    {
        ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousEmissiveMedium>(Vec4f(0.5f, 0.25f, 0.125f));

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vec4f(-12.0f, 5.001f, 15.0f)));
        scene.AddObject(std::move(object));
    }
    */

    /*
    // heterogeneous absorption medium
    {
        auto densityBitmap = helpers::LoadBitmapObject(gOptions.dataPath, "TEXTURES/Volume/Clouds/wdas_cloud_half.vdb");
        //auto densityBitmap = helpers::LoadBitmapObject(gOptions.dataPath, "TEXTURES/Volume/sphere.vdb");

        const float density = 20.0f;
        const Vec4f scattering = Vec4f(1.0f, 1.0f, 1.0f) * density;
        const Vec4f attenuation = Vec4f(0.0f, 0.0f, 0.0f) * density;

        const Vec4f extintion = attenuation + scattering;
        const Vec4f scatteringAlbedo(0.65f, 0.53f, 0.351f);

        const Plane plane(Vec4f(0.0f, 1.0f, 0.0f), Vec4f(0.0f, 0.0f, 0.0f));
        TexturePtr texture = MakeSharedPtr<BitmapTexture3D>(densityBitmap);
        //TexturePtr texture = MakeSharedPtr<NoiseTexture3D>(Vec4f(0.0f, 0.0f, 0.0f), Vec4f(1.0f, 1.0f, 1.0f), 5);
        //TexturePtr texture = MakeSharedPtr<CheckerboardTexture>(Vec4f(0.0f, 0.0f, 0.0f), Vec4f(1.0f, 1.0f, 1.0f));
        //TexturePtr texture = MakeSharedPtr<GradientTexture>(Vec4f(0.1f, 0.2, 0.3f), Vec4f(0.0f), plane, 11.0f);
        ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.25f, 0.85f, 1.53f));

        //MediumPtr medium = MakeUniquePtr<HeterogeneousAbsorptiveMedium>(texture, extintion);
        //MediumPtr medium = MakeUniquePtr<HomogenousAbsorptiveMedium>(Vec4f(1.0f, 0.5f, 0.25f));
        MediumPtr medium = MakeUniquePtr<HeterogeneousScatteringMedium>(texture, extintion, scatteringAlbedo);

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vec4f(0.0f, 0.0f, 0.0f)));
        scene.AddObject(std::move(object));
    }
    */

    /*
    // emissive sphere
    {
        auto material = Material::Create();
        material->debugName = "default";
        material->SetBsdf("diffuse");
        material->baseColor.baseValue = HdrColorRGB();
        material->emission.baseValue = HdrColorRGB(10.0f, 10.0f, 10.0f);
        material->Compile();

        ShapePtr shape = MakeUniquePtr<SphereShape>(1.0f);
        ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        instance->BindMaterial(material);
        instance->SetTransform(Matrix4::MakeTranslation(Vec4f(6.0f, 5.0f, 0.0f)));
        scene.AddObject(std::move(instance));
    }
    */

    //{
    //    auto material = Material::Create();
    //    material->debugName = "default";
    //    material->SetBsdf("diffuse");
    //    material->baseColor = Vec4f(0.9f);
    //    material->baseColor.texture = bitmapTextureA;
    //    material->Compile();

    //    ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f));
    //    ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
    //    instance->BindMaterial(material);
    //    instance->SetTransform(Matrix4::MakeTranslation(Vec4f(-2.4f, 1.0f, 0.0f)));
    //    scene.AddObject(std::move(instance));
    //}

    //{
    //    auto material = Material::Create();
    //    material->debugName = "default";
    //    material->SetBsdf("diffuse");
    //    material->baseColor = Vec4f(0.9f);
    //    material->baseColor.texture = bitmapTextureB;
    //    material->Compile();

    //    ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f));
    //    ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
    //    instance->BindMaterial(material);
    //    instance->SetTransform(Matrix4::MakeTranslation(Vec4f(2.4f, 1.0f, 0.0f)));
    //    scene.AddObject(std::move(instance));
    //}

    //{
    //    auto material = Material::Create();
    //    material->debugName = "default";
    //    material->SetBsdf("diffuse");
    //    material->baseColor = Vec4f(0.9f);
    //    material->baseColor.texture = noiseTexture;
    //    material->Compile();

    //    ShapePtr shape = MakeUniquePtr<BoxShape>(Vec4f(1.0f));
    //    ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
    //    instance->BindMaterial(material);
    //    instance->SetTransform(Matrix4::MakeTranslation(Vec4f(0.0f, 3.5f, 0.0f)));
    //    scene.AddObject(std::move(instance));
    //}

/*
    {
        const Vec4f lightColor(500.0f, 400.0f, 300.0f);
        const Vec4f lightDirection(1.1f, -0.7f, 0.9f);
        auto light = MakeUniquePtr<DirectionalLight>(lightColor, 0.05f);
        auto lightObject = MakeUniquePtr<LightSceneObject>(std::move(light));
        lightObject->SetTransform(Quaternion::FromEulerAngles(Vec3f(0.3f, -1.9f, 0.0f)).ToMatrix());
        scene.AddObject(std::move(lightObject));
    }
*/

    /*
    // test decal A
    {
        UniquePtr<DecalSceneObject> decal = MakeUniquePtr<DecalSceneObject>();
        decal->SetTransform(Quaternion::FromAxisAndAngle(VECTOR_X, NFE_MATH_PI / 2.0f).ToMatrix() * Matrix4::MakeScaling(Vec4f(5.0f, 1.0f, 5.0f)));
        decal->baseColor.texture = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/Decals/CityStreetRoadAsphaltRepairPatch005/CityStreetRoadAsphaltRepairPatch005_ALPHAMASKED_4K.DDS");
        decal->roughness.texture = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/Decals/CityStreetRoadAsphaltRepairPatch005/CityStreetRoadAsphaltRepairPatch005_ROUGHNESS_4K.DDS");
        decal->order = 0;

        scene.AddObject(std::move(decal));
    }

    // test decal B
    for (uint32 i = 0; i < 10; ++i)
    {
        for (uint32 j = 0; j < 10; ++j)
        {
            UniquePtr<DecalSceneObject> decal = MakeUniquePtr<DecalSceneObject>();
            decal->SetTransform(
                Quaternion::FromAxisAndAngle(VECTOR_X, NFE_MATH_PI / 2.0f).ToMatrix() *
                Matrix4::MakeTranslation(Vec4f(3.0f * i, 0.0f, 3.0f * j)) *
                Matrix4::MakeScaling(Vec4f(0.5f, 1.0f, 0.5f)));
            //decal->baseColor.baseValue = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
            decal->baseColor.texture = bitmapTextureA;
            decal->baseColor.baseValue = Vec4f(1.0f, 0.5f, 0.5f, 1.0f);
            decal->alphaMin = 1.0f;
            decal->alphaMax = 1.0f;
            decal->order = 1;

            scene.AddObject(std::move(decal));
        }
    }
    */

    {
        const HdrColorRGB lightColor(0.5f, 0.5f, 0.5f);
        auto background = MakeUniquePtr<BackgroundLight>(lightColor);
        background->SetTexture(backgroundTexture);
        auto lightObject = MakeUniquePtr<LightSceneObject>(std::move(background));
        scene.AddObject(std::move(lightObject));
    }

    {
        Transform transform(Vec4f(-5.4f, 5.0f, 5.1f), Quaternion::FromEulerAngles(Vec3f(DegToRad(30.9f), DegToRad(141.0f), 0.0f)));
        camera.SetTransform(transform);
        camera.SetPerspective(1.0f, DegToRad(60.0f));
    }

    return true;
}

} // namespace helpers
} // namespace NFE