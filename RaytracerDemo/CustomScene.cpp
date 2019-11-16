#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "../nfEngine/Raytracer/Textures/NoiseTexture.h"
#include "../nfEngine/Raytracer/Textures/NoiseTexture3D.h"
#include "../nfEngine/Raytracer/Textures/CheckerboardTexture.h"
#include "../nfEngine/Raytracer/Textures/GradientTexture.h"
#include "../nfEngine/Raytracer/Textures/MixTexture.h"
#include "../nfEngine/Raytracer/Textures/BitmapTexture3D.h"

#include "../nfEngine/Raytracer/Scene/Light/DirectionalLight.h"
#include "../nfEngine/Raytracer/Scene/Light/BackgroundLight.h"
#include "../nfEngine/Raytracer/Scene/Light/PointLight.h"

#include "../nfEngine/Raytracer/Scene/Object/SceneObject_Shape.h"
#include "../nfEngine/Raytracer/Scene/Object/SceneObject_Light.h"
#include "../nfEngine/Raytracer/Scene/Object/SceneObject_Decal.h"

#include "../nfEngine/Raytracer/Shapes/RectShape.h"
#include "../nfEngine/Raytracer/Shapes/BoxShape.h"
#include "../nfEngine/Raytracer/Shapes/SphereShape.h"

#include "../nfEngine/Raytracer/Medium/Medium.h"

namespace NFE {
namespace helpers {

using namespace RT;
using namespace Math;
using namespace Common;

bool LoadCustomScene(Scene& scene, Camera& camera)
{
    auto bitmapTextureA = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/default.bmp");
    auto backgroundTexture = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/ENV/OutdoorCityParkingLotEveningClear_4K.exr");
    //auto bitmapTextureB = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/Portal/dirty4x4.bmp");
    //auto noiseTexture = std::shared_ptr<ITexture>(new NoiseTexture(Vector4(1.0f), Vector4(0.0f)));
    //auto texture = std::shared_ptr<ITexture>(new MixTexture(bitmapTextureA, bitmapTextureB, noiseTexture));

    // floor
    /*
    {
        auto material = Material::Create();
        material->debugName = "floor";
        material->SetBsdf(String("diffuse"));
        material->baseColor = Math::HdrColorRGB(0.9f, 0.9f, 0.9f);
        material->baseColor.texture = bitmapTextureA;
        //material->emission = Math::Vector4(4.0f, 4.0f, 4.0f);
        //material->emission.texture = emissionTexture;
        //material->baseColor.texture = helpers::LoadTexture(gOptions.dataPath, "TEXTURES/default.bmp");
        material->roughness = 0.2f;
        material->Compile();

        const Float2 size(1000.0f, 1000.0f);
        const Float2 texScale(0.2f, 0.2f);
        auto rect = MakeSharedPtr<RectShape>(size, texScale);
        UniquePtr<ShapeSceneObject> instance = MakeUniquePtr<ShapeSceneObject>(std::move(rect));
        instance->BindMaterial(material);
        instance->SetTransform(Quaternion::FromEulerAngles(Float3(-NFE_MATH_PI / 2.0f, -0.01f, 0.0f)).ToMatrix());
        scene.AddObject(std::move(instance));
    }
    */

    /*
    Random random;

    for (int32 i = 0; i < 2000; ++i)
    {
        auto material = Material::Create();
        material->debugName = "default";
        material->SetBsdf("diffuse");
        material->baseColor = Vector4(1.0f);
        material->roughness = random.GetFloat() * 0.6f;
        material->Compile();

        const Vector4 size = Vector4(0.5f, 0.5f, 0.5f) + random.GetVector4() * Vector4(5.0f, 10.0f, 5.0f);
        const Vector4 pos = random.GetVector4Bipolar() * 1000.0f;

        const Matrix4 translationMatrix = Matrix4::MakeTranslation(Vector4(pos.x, size.y / 2.0f, pos.y));
        const Matrix4 rotationMatrix = Quaternion::RotationY(pos.z * RT_2PI).ToMatrix();

        SceneObjectPtr instance = MakeUniquePtr<BoxSceneObject>(size);
        instance->BindMaterial(material);
        instance->SetTransform(rotationMatrix * translationMatrix);
        scene.AddObject(std::move(instance));
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

        ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(0.2f, 5.0f, 5.0f));
        ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        instance->BindMaterial(material);
        instance->SetTransform(Matrix4::MakeTranslation(Vector4(10.0f, 5.0, 0.0f)));
        scene.AddObject(std::move(instance));
    }
    */

    /*
    // homogenous scattering medium
    {
        const float density = 0.5f;
        const Vector4 scattering = Vector4(0.25f, 0.5f, 1.0f) * density;
        const Vector4 attenuation = Vector4(0.0f, 0.0f, 0.0f) * density;

        const Vector4 extintion = attenuation + scattering;
        const Vector4 scatteringAlbedo = scattering / extintion;

        ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousScatteringMedium>(extintion, scatteringAlbedo);

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vector4(0.0f, 5.001f, 0.0f)));
        scene.AddObject(std::move(object));
    }

    // homogenous absorption medium
    {
        const float density = 1.0f;
        const Vector4 attenuation = Vector4(0.25f, 0.5f, 1.0f) * density;

        ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousAbsorptiveMedium>(attenuation);

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vector4(-12.0f, 5.001f, 0.0f)));
        scene.AddObject(std::move(object));
    }

    // homogenous emissive medium
    {
        ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousEmissiveMedium>(Vector4(0.125f, 0.25f, 0.5f));

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vector4(12.0f, 5.001f, 0.0f)));
        scene.AddObject(std::move(object));
    }


    // homogenous scattering medium
    {
        const float density = 4.0f;
        const Vector4 scattering = Vector4(0.25f, 0.5f, 1.0f) * density;
        const Vector4 attenuation = Vector4(0.0f, 0.0f, 0.0f) * density;

        const Vector4 extintion = attenuation + scattering;
        const Vector4 scatteringAlbedo = scattering / extintion;

        ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousScatteringMedium>(extintion, scatteringAlbedo);

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vector4(0.0f, 5.001f, 15.0f)));
        scene.AddObject(std::move(object));
    }

    // homogenous absorption medium
    {
        const float density = 1.0f;
        const Vector4 attenuation = Vector4(1.0f, 0.5f, 0.25f) * density;

        ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousAbsorptiveMedium>(attenuation);

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vector4(12.0f, 5.001f, 15.0f)));
        scene.AddObject(std::move(object));
    }

    // homogenous emissive medium
    {
        ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(5.0f, 5.0f, 5.0f));
        MediumPtr medium = MakeUniquePtr<HomogenousEmissiveMedium>(Vector4(0.5f, 0.25f, 0.125f));

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vector4(-12.0f, 5.001f, 15.0f)));
        scene.AddObject(std::move(object));
    }
    */

    // heterogenous absorption medium
    {
        auto densityBitmap = helpers::LoadBitmapObject(gOptions.dataPath, "TEXTURES/Volume/Clouds/wdas_cloud_half.vdb");
        //auto densityBitmap = helpers::LoadBitmapObject(gOptions.dataPath, "TEXTURES/Volume/sphere.vdb");

        const float density = 20.0f;
        const Vector4 scattering = Vector4(1.0f, 1.0f, 1.0f) * density;
        const Vector4 attenuation = Vector4(0.0f, 0.0f, 0.0f) * density;

        const Vector4 extintion = attenuation + scattering;
        const Vector4 scatteringAlbedo(0.65f, 0.53f, 0.351f);

        const Plane plane(Vector4(0.0f, 1.0f, 0.0f), Vector4(0.0f, 0.0f, 0.0f));
        TexturePtr texture = MakeSharedPtr<BitmapTexture3D>(densityBitmap);
        //TexturePtr texture = MakeSharedPtr<NoiseTexture3D>(Vector4(0.0f, 0.0f, 0.0f), Vector4(1.0f, 1.0f, 1.0f), 5);
        //TexturePtr texture = MakeSharedPtr<CheckerboardTexture>(Vector4(0.0f, 0.0f, 0.0f), Vector4(1.0f, 1.0f, 1.0f));
        //TexturePtr texture = MakeSharedPtr<GradientTexture>(Vector4(0.1f, 0.2, 0.3f), Vector4(0.0f), plane, 11.0f);
        ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(1.25f, 0.85f, 1.53f));

        //MediumPtr medium = MakeUniquePtr<HeterogeneousAbsorptiveMedium>(texture, extintion);
        //MediumPtr medium = MakeUniquePtr<HomogenousAbsorptiveMedium>(Vector4(1.0f, 0.5f, 0.25f));
        MediumPtr medium = MakeUniquePtr<HeterogeneousScatteringMedium>(texture, extintion, scatteringAlbedo);

        auto object = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
        object->BindMedium(medium);
        object->SetTransform(Matrix4::MakeTranslation(Vector4(0.0f, 0.0f, 0.0f)));
        scene.AddObject(std::move(object));
    }

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
        instance->SetTransform(Matrix4::MakeTranslation(Vector4(6.0f, 5.0f, 0.0f)));
        scene.AddObject(std::move(instance));
    }
    */

    //{
    //    auto material = Material::Create();
    //    material->debugName = "default";
    //    material->SetBsdf("diffuse");
    //    material->baseColor = Vector4(0.9f);
    //    material->baseColor.texture = bitmapTextureA;
    //    material->Compile();

    //    ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(1.0f));
    //    ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
    //    instance->BindMaterial(material);
    //    instance->SetTransform(Matrix4::MakeTranslation(Vector4(-2.4f, 1.0f, 0.0f)));
    //    scene.AddObject(std::move(instance));
    //}

    //{
    //    auto material = Material::Create();
    //    material->debugName = "default";
    //    material->SetBsdf("diffuse");
    //    material->baseColor = Vector4(0.9f);
    //    material->baseColor.texture = bitmapTextureB;
    //    material->Compile();

    //    ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(1.0f));
    //    ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
    //    instance->BindMaterial(material);
    //    instance->SetTransform(Matrix4::MakeTranslation(Vector4(2.4f, 1.0f, 0.0f)));
    //    scene.AddObject(std::move(instance));
    //}

    //{
    //    auto material = Material::Create();
    //    material->debugName = "default";
    //    material->SetBsdf("diffuse");
    //    material->baseColor = Vector4(0.9f);
    //    material->baseColor.texture = noiseTexture;
    //    material->Compile();

    //    ShapePtr shape = MakeUniquePtr<BoxShape>(Vector4(1.0f));
    //    ShapeSceneObjectPtr instance = MakeUniquePtr<ShapeSceneObject>(std::move(shape));
    //    instance->BindMaterial(material);
    //    instance->SetTransform(Matrix4::MakeTranslation(Vector4(0.0f, 3.5f, 0.0f)));
    //    scene.AddObject(std::move(instance));
    //}

    //{
    //    const Vector4 lightColor(500.0f, 400.0f, 300.0f);
    //    const Vector4 lightDirection(1.1f, -0.7f, 0.9f);
    //    auto light = MakeUniquePtr<DirectionalLight>(lightDirection, lightColor, 0.15f);
    //    auto lightObject = MakeUniquePtr<LightSceneObject>(std::move(light));
    //    scene.AddObject(std::move(lightObject));
    //}

    /*
    // test decal A
    {
        UniquePtr<DecalSceneObject> decal = MakeUniquePtr<DecalSceneObject>();
        decal->SetTransform(Quaternion::FromAxisAndAngle(VECTOR_X, NFE_MATH_PI / 2.0f).ToMatrix() * Matrix4::MakeScaling(Vector4(5.0f, 1.0f, 5.0f)));
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
                Matrix4::MakeTranslation(Vector4(3.0f * i, 0.0f, 3.0f * j)) *
                Matrix4::MakeScaling(Vector4(0.5f, 1.0f, 0.5f)));
            //decal->baseColor.baseValue = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            decal->baseColor.texture = bitmapTextureA;
            decal->baseColor.baseValue = Vector4(1.0f, 0.5f, 0.5f, 1.0f);
            decal->alphaMin = 1.0f;
            decal->alphaMax = 1.0f;
            decal->order = 1;

            scene.AddObject(std::move(decal));
        }
    }
    */

    {
        const HdrColorRGB lightColor(1.0f, 0.7f, 0.5f);
        auto background = MakeUniquePtr<BackgroundLight>(lightColor);
        background->mTexture = backgroundTexture;
        auto lightObject = MakeUniquePtr<LightSceneObject>(std::move(background));
        scene.AddObject(std::move(lightObject));
    }
    //{
    //    const HdrColorRGB lightColor(2.0f, 2.0f, 2.0f);
    //    auto background = MakeUniquePtr<BackgroundLight>(lightColor);
    //    background->mTexture = backgroundTexture;
    //    auto lightObject = MakeUniquePtr<LightSceneObject>(std::move(background));
    //    scene.AddObject(std::move(lightObject));
    //}

    {
        Transform transform(Vector4(-14.4f, -4.9f, 3.1f), Quaternion::FromEulerAngles(Float3(DegToRad(-17.9f), DegToRad(101.0f), 0.0f)));
        camera.SetTransform(transform);
        camera.SetPerspective(1.0f, DegToRad(8.0f));
    }

    return true;
}

} // namespace helpers
} // namespace NFE