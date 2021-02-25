#include "PCH.h"
#include "Demo.h"
#include "MeshLoader.h"

#include "Engine/Raytracer/Scene/Light/PointLight.h"
#include "Engine/Raytracer/Scene/Light/AreaLight.h"
#include "Engine/Raytracer/Scene/Light/BackgroundLight.h"
#include "Engine/Raytracer/Scene/Light/DirectionalLight.h"
#include "Engine/Raytracer/Scene/Light/SpotLight.h"
#include "Engine/Raytracer/Scene/Object/SceneObject_Shape.h"
#include "Engine/Raytracer/Scene/Object/SceneObject_Light.h"
#include "Engine/Raytracer/Shapes/BoxShape.h"
#include "Engine/Raytracer/Shapes/SphereShape.h"
#include "Engine/Raytracer/Shapes/CylinderShape.h"
#include "Engine/Raytracer/Shapes/RectShape.h"
#include "Engine/Raytracer/Shapes/CsgShape.h"
#include "Engine/Raytracer/Color/ColorRGB.h"
#include "Engine/Raytracer/Textures/CheckerboardTexture.h"
#include "Engine/Raytracer/Textures/BitmapTexture.h"
#include "Engine/Raytracer/Textures/NoiseTexture.h"
#include "Engine/Raytracer/Textures/MixTexture.h"
#include "Engine/Raytracer/Medium/Medium.h"

#include "Engine/Common/Logger/Logger.hpp"

#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"

namespace NFE {
namespace helpers {

using namespace RT;
using namespace Math;
using namespace Common;

using TexturesMap = HashMap<String, TexturePtr>;

static bool ParseVector2(const rapidjson::Value& value, Vec4f& outVector)
{
    if (!value.IsArray())
    {
        NFE_LOG_ERROR("2D vector description must be an array");
        return false;
    }

    if (value.Size() != 2)
    {
        NFE_LOG_ERROR("Invalid array size for 2D vector");
        return false;
    }

    const float x = static_cast<float>(value[0u].GetDouble());
    const float y = static_cast<float>(value[1u].GetDouble());

    outVector = Vec4f(x, y, 0.0f, 0.0f);

    return true;
}

static bool ParseVector3(const rapidjson::Value& value, Vec4f& outVector)
{
    if (!value.IsArray())
    {
        NFE_LOG_ERROR("3D vector description must be an array");
        return false;
    }

    if (value.Size() != 3)
    {
        NFE_LOG_ERROR("Invalid array size for 3D vector");
        return false;
    }

    const float x = static_cast<float>(value[0u].GetDouble());
    const float y = static_cast<float>(value[1u].GetDouble());
    const float z = static_cast<float>(value[2u].GetDouble());

    outVector = Vec4f(x, y, z, 0.0f);

    return true;
}

static bool ParseHdrColorRGB(const rapidjson::Value& value, HdrColorRGB& outColor)
{
    if (!value.IsArray())
    {
        NFE_LOG_ERROR("RGB color description must be an array");
        return false;
    }

    if (value.Size() != 3)
    {
        NFE_LOG_ERROR("Invalid array size for RGB color");
        return false;
    }

    const float r = static_cast<float>(value[0u].GetDouble());
    const float g = static_cast<float>(value[1u].GetDouble());
    const float b = static_cast<float>(value[2u].GetDouble());

    outColor = HdrColorRGB(r, g, b);

    return true;
}

static bool TryParseBool(const rapidjson::Value& value, const char* name, bool optional, bool& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            NFE_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    if (!value[name].IsBool())
    {
        NFE_LOG_ERROR("Property '%s' must be a bool", name);
        return false;
    }

    outValue = value[name].GetBool();
    return true;
}

static bool TryParseInt(const rapidjson::Value& value, const char* name, bool optional, int32& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            NFE_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    if (!value[name].IsInt())
    {
        NFE_LOG_ERROR("Property '%s' must be an integer", name);
        return false;
    }

    outValue = value[name].GetInt();
    return true;
}

static bool TryParseFloat(const rapidjson::Value& value, const char* name, bool optional, float& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            NFE_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    if (!value[name].IsDouble())
    {
        NFE_LOG_ERROR("Property '%s' must be a float", name);
        return false;
    }

    outValue = static_cast<float>(value[name].GetDouble());
    return true;
}

static bool TryParseVector2(const rapidjson::Value& value, const char* name, bool optional, Vec4f& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            NFE_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    return ParseVector2(value[name], outValue);
}

static bool TryParseVector3(const rapidjson::Value& value, const char* name, bool optional, Vec4f& outValue)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            NFE_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    return ParseVector3(value[name], outValue);
}

static bool TryParseHdrColorRGB(const rapidjson::Value& value, const char* name, bool optional, HdrColorRGB& outColor)
{
    if (!value.HasMember(name))
    {
        if (optional)
        {
            return true;
        }
        else
        {
            NFE_LOG_ERROR("Missing '%hs' property", name);
            return false;
        }
    }

    return ParseHdrColorRGB(value[name], outColor);
}

static bool TryParseTransform(const rapidjson::Value& parentValue, const char* name, Transform& outValue)
{
    if (!parentValue.HasMember(name))
    {
        return true;
    }

    const rapidjson::Value& value = parentValue[name];
    if (!value.IsObject())
    {
        NFE_LOG_ERROR("Transform description must be a structure");
        return false;
    }

    Vec4f translation = Vec4f::Zero();
    if (!TryParseVector3(value, "translation", true, translation))
        return false;

    Vec4f orientation = Vec4f::Zero();
    if (!TryParseVector3(value, "orientation", true, orientation))
        return false;

    orientation *= (NFE_MATH_PI / 180.0f);

    outValue = Transform(translation, Quaternion::FromEulerAngles(orientation.ToVec3f()));

    return true;
}

static bool TryParseTextureName(const rapidjson::Value& value, const char* name, const TexturesMap& textures, TexturePtr& outValue)
{
    if (!value.HasMember(name))
    {
        return true;
    }

    if (!value[name].IsString())
    {
        NFE_LOG_ERROR("Texture path '%s' must be a string", name);
        return false;
    }

    const String textureName{ value[name].GetString() };
    const auto iter = textures.Find(textureName);
    if (iter != textures.End())
    {
        outValue = iter->second;
        return true;
    }

    outValue = helpers::LoadTexture(gOptions.dataPath, textureName);
    return true;
}

static bool TryParseMaterialName(const MaterialsMap& materials, const rapidjson::Value& value, const char* name, MaterialPtr& outValue)
{
    if (!value.HasMember(name))
    {
        return true;
    }

    if (!value[name].IsString())
    {
        NFE_LOG_ERROR("Material name '%s' must be a string", name);
        return false;
    }

    const String materialName{ value[name].GetString() };
    const auto iter = materials.Find(materialName);
    if (iter == materials.end())
    {
        NFE_LOG_ERROR("Material '%s' does not exist", materialName.Str());
        return false;
    }

    outValue = iter->second;
    return true;
}

static TexturePtr ParseTexture(const rapidjson::Value& value, const TexturesMap& textures, String& outName)
{
    if (!value.IsObject())
    {
        NFE_LOG_ERROR("Texture description must be a structure");
        return nullptr;
    }

    if (!value.HasMember("name"))
    {
        NFE_LOG_ERROR("Texture is missing 'name' field");
        return nullptr;
    }

    const String name{ value["name"].GetString() };
    if (name.Empty())
    {
        NFE_LOG_ERROR("Texture name cannot be empty");
        return nullptr;
    }
    outName = name;

    if (!value.HasMember("type"))
    {
        NFE_LOG_ERROR("Texture is missing 'type' field");
        return nullptr;
    }

    const String type{ value["type"].GetString() };
    if (type.Empty())
    {
        NFE_LOG_ERROR("Texture type cannot be empty");
        return nullptr;
    }

    if (type == "bitmap")
    {
        if (!value.HasMember("path"))
        {
            NFE_LOG_ERROR("Texture is missing 'path' field");
            return nullptr;
        }

        const String path{ value["path"].GetString() };
        if (type.Empty())
        {
            NFE_LOG_ERROR("Texture path cannot be empty");
            return nullptr;
        }

        BitmapPtr bitmap = LoadBitmapObject(gOptions.dataPath, path);
        if (!bitmap || bitmap->GetWidth() == 0 || bitmap->GetHeight() == 0)
        {
            return nullptr;
        }

        return MakeSharedPtr<BitmapTexture>(bitmap);
    }
    else if (type == "checkerboard")
    {
        Vec4f colorA = Vec4f::Zero();
        Vec4f colorB = Vec4f::Zero();
        if (!TryParseVector3(value, "colorA", false, colorA)) return nullptr;
        if (!TryParseVector3(value, "colorB", false, colorB)) return nullptr;

        return MakeSharedPtr<CheckerboardTexture>(colorA, colorB);
    }
    else if (type == "noise")
    {
        Vec4f colorA = Vec4f::Zero();
        Vec4f colorB = Vec4f::Zero();
        if (!TryParseVector3(value, "colorA", false, colorA)) return nullptr;
        if (!TryParseVector3(value, "colorB", false, colorB)) return nullptr;

        int32 numOctaves = 1;
        if (!TryParseInt(value, "octaves", true, numOctaves)) return nullptr;
        numOctaves = Clamp(numOctaves, 1, 20);

        return MakeSharedPtr<NoiseTexture>(colorA, colorB, static_cast<uint32>(numOctaves));
    }
    else if (type == "mix")
    {
        TexturePtr texA, texB, texWeight;

        if (!TryParseTextureName(value, "textureA", textures, texA)) return nullptr;
        if (!TryParseTextureName(value, "textureB", textures, texB)) return nullptr;
        if (!TryParseTextureName(value, "weight", textures, texWeight)) return nullptr;

        return MakeSharedPtr<MixTexture>(texA, texB, texWeight);
    }

    NFE_LOG_ERROR("Invalid texture type name: '%s'", type.Str());
    return nullptr;
}

static MaterialPtr ParseMaterial(const rapidjson::Value& value, const TexturesMap& textures)
{
    if (!value.IsObject())
    {
        NFE_LOG_ERROR("Material description must be a structure");
        return nullptr;
    }

    if (!value.HasMember("name"))
    {
        NFE_LOG_ERROR("Material is missing 'name' field");
        return nullptr;
    }

    const String name{ value["name"].GetString() };
    if (name.Empty())
    {
        NFE_LOG_ERROR("Material name cannot be empty");
        return nullptr;
    }

    String bsdfName{ Material::DefaultBsdfName };
    if (value.HasMember("bsdf"))
    {
        bsdfName = value["bsdf"].GetString();
    }

    MaterialPtr material = Material::Create();
    material->debugName = std::move(name);
    material->SetBsdf(bsdfName);

    if (!TryParseBool(value, "dispersive", true, material->dispersion.enable)) return nullptr;

    {
        HdrColorRGB color;
        if (TryParseHdrColorRGB(value, "baseColor", true, color))
        {
            material->baseColor.SetBaseValue(MakeSharedPtr<ColorRGB>(color));
        }
        else
        {
            return nullptr;
        }
    }

    {
        HdrColorRGB color;
        if (TryParseHdrColorRGB(value, "emissionColor", true, color))
        {
            material->emission.SetBaseValue(MakeSharedPtr<ColorRGB>(color));
        }
        else
        {
            return nullptr;
        }
    }

    {
        TexturePtr texture;
        if (TryParseTextureName(value, "baseColorTexture", textures, texture))
        {
            material->baseColor.SetTexture(texture);
        }
        else
        {
            return nullptr;
        }
    }

    {
        TexturePtr texture;
        if (TryParseTextureName(value, "emissionTexture", textures, texture))
        {
            material->emission.SetTexture(texture);
        }
        else
        {
            return nullptr;
        }
    }

    if (!TryParseFloat(value, "roughness", true, material->roughness.baseValue)) return nullptr;
    if (!TryParseFloat(value, "metalness", true, material->metalness.baseValue)) return nullptr;

    if (!TryParseTextureName(value, "roughnessTexture", textures, material->roughness.texture)) return nullptr;
    if (!TryParseTextureName(value, "metalnessTexture", textures, material->metalness.texture)) return nullptr;
    if (!TryParseTextureName(value, "normalMap", textures, material->normalMap)) return nullptr;
    if (!TryParseTextureName(value, "maskMap", textures, material->maskMap)) return nullptr;

    if (!TryParseFloat(value, "normalMapStrength", true, material->normalMapStrength)) return nullptr;
    if (!TryParseFloat(value, "IoR", true, material->IoR)) return nullptr;
    if (!TryParseFloat(value, "K", true, material->K)) return nullptr;

    material->Compile();

    return material;
}

static ShapePtr ParseShape(const rapidjson::Value& value, MaterialsMap& materials)
{
    ShapePtr shape;

    if (!value.HasMember("type"))
    {
        NFE_LOG_ERROR("Object is missing 'type' field");
        return nullptr;
    }

    // parse type
    const String typeStr{ value["type"].GetString() };
    if (typeStr == "sphere")
    {
        float radius = 1.0f;
        if (!TryParseFloat(value, "radius", false, radius))
        {
            return nullptr;
        }

        shape = MakeUniquePtr<SphereShape>(radius);
    }
    else if (typeStr == "cylinder")
    {
        float radius = 0.5f;
        if (!TryParseFloat(value, "radius", false, radius))
        {
            return nullptr;
        }

        float height = 1.0f;
        if (!TryParseFloat(value, "height", false, height))
        {
            return nullptr;
        }

        shape = MakeUniquePtr<CylinderShape>(radius, height);
    }
    else if (typeStr == "box")
    {
        Vec4f size;
        if (!TryParseVector3(value, "size", false, size))
        {
            return nullptr;
        }

        shape = MakeUniquePtr<BoxShape>(size);
    }
    else if (typeStr == "rect" || typeStr == "plane")
    {
        Vec4f size(FLT_MAX);
        if (!TryParseVector2(value, "size", false, size))
        {
            return nullptr;
        }
        Vec4f textureScale(1.0f);
        if (!TryParseVector2(value, "textureScale", true, textureScale))
        {
            return nullptr;
        }

        shape = MakeUniquePtr<RectShape>(size.ToVec2f(), textureScale.ToVec2f());
    }
    else if (typeStr == "csg")
    {
        // TODO parse CSG structure
        shape = MakeUniquePtr<CsgShape>();
    }
    else if (typeStr == "mesh")
    {
        if (!value.HasMember("path"))
        {
            NFE_LOG_ERROR("Missing 'path' property");
            return nullptr;
        }

        if (!value["path"].IsString())
        {
            NFE_LOG_ERROR("Mesh path must be a string");
            return nullptr;
        }

        float scale = 1.0f;
        if (!TryParseFloat(value, "scale", true, scale))
        {
            return nullptr;
        }

        const String path = gOptions.dataPath + value["path"].GetString();
        shape = helpers::LoadMesh(path, materials, scale);
    }
    else
    {
        NFE_LOG_ERROR("Unknown scene object type: '%s'", typeStr.Str());
    }

    return shape;
}

static bool ParseLight(const rapidjson::Value& value, Scene& scene, const TexturesMap& textures)
{
    if (!value.IsObject())
    {
        NFE_LOG_ERROR("Light description must be a structure");
        return false;
    }

    if (!value.HasMember("type"))
    {
        NFE_LOG_ERROR("Light is missing 'type' field");
        return false;
    }

    HdrColorRGB lightColor;
    if (!TryParseHdrColorRGB(value, "color", false, lightColor))
    {
        return false;
    }

    LightPtr light;

    // parse type
    const String typeStr{ value["type"].GetString() };
    if (typeStr == "area")
    {
        if (!value.HasMember("shape"))
        {
            NFE_LOG_ERROR("Area light is missing 'shape' field");
            return false;
        }

        MaterialsMap materials;
        ShapePtr shape = ParseShape(value["shape"], materials);
        auto areaLight = MakeUniquePtr<AreaLight>(std::move(shape), lightColor);

        if (!TryParseTextureName(value, "texture", textures, areaLight->mTexture))
            return false;

        if (areaLight->mTexture && !areaLight->mTexture->IsSamplable())
        {
            areaLight->mTexture->MakeSamplable();
        }


        light = std::move(areaLight);
    }
    else if (typeStr == "point")
    {
        auto pointLight = MakeUniquePtr<PointLight>(lightColor);
        // TODO texture
        light = std::move(pointLight);
    }
    else if (typeStr == "spot")
    {
        float angle = 0.0f;
        if (!TryParseFloat(value, "angle", true, angle))
        {
            return false;
        }
        const float angleRad = angle / 180.0f * NFE_MATH_PI;

        auto spotLight = MakeUniquePtr<SpotLight>(lightColor, angleRad);
        // TODO texture
        light = std::move(spotLight);
    }
    else if (typeStr == "directional")
    {
        float angle = 0.0f;
        if (!TryParseFloat(value, "angle", true, angle))
        {
            return false;
        }

        auto dirLight = MakeUniquePtr<DirectionalLight>(lightColor, DegToRad(angle));
        // TODO texture
        light = std::move(dirLight);
    }
    else if (typeStr == "background")
    {
        auto backgroundLight = MakeUniquePtr<BackgroundLight>(lightColor);

        TexturePtr texture;
        if (!TryParseTextureName(value, "texture", textures, texture))
            return false;

        backgroundLight->SetTexture(texture);

        light = std::move(backgroundLight);
    }
    else if (typeStr == "sphere") // TODO merge with "area"
    {
        float radius = 0.0f;
        if (!TryParseFloat(value, "radius", false, radius))
        {
            return false;
        }

        auto shape = MakeSharedPtr<SphereShape>(radius);
        auto areaLight = MakeUniquePtr<AreaLight>(std::move(shape), lightColor);
    }
    else
    {
        NFE_LOG_ERROR("Unknown light type: '%s'", typeStr.Str());
        return false;
    }

    auto lightObject = MakeUniquePtr<LightSceneObject>(std::move(light));

    {
        Transform transform;
        if (!TryParseTransform(value, "transform", transform))
        {
            return false;
        }
        lightObject->SetTransform(transform.ToMatrix());
    }

    scene.AddObject(std::move(lightObject));

    return true;
}

static bool ParseObject(const rapidjson::Value& value, Scene& scene, MaterialsMap& materials)
{
    if (!value.IsObject())
    {
        NFE_LOG_ERROR("Object description must be a structure");
        return false;
    }

    ShapePtr shape = ParseShape(value, materials);
    if (!shape)
    {
        return false;
    }

    ShapeSceneObjectPtr sceneObject = MakeUniquePtr<ShapeSceneObject>(std::move(shape));

    // TODO velocity

    MaterialPtr material;
    if (!TryParseMaterialName(materials, value, "material", material))
        return false;
    sceneObject->BindMaterial(material);

    Transform transform;
    if (!TryParseTransform(value, "transform", transform))
        return false;
    sceneObject->SetTransform(transform.ToMatrix());

    scene.AddObject(std::move(sceneObject));
    return true;
}

static bool ParseCamera(const rapidjson::Value& value, const TexturesMap& textures, RT::Camera& camera)
{
    if (!value.IsObject())
    {
        NFE_LOG_ERROR("Light description must be a structure");
        return false;
    }

    Math::Transform transform;
    if (!TryParseTransform(value, "transform", transform))
        return false;

    float fov = 60.0f;
    if (!TryParseFloat(value, "fieldOfView", true, fov))
        return false;

    camera.SetTransform(transform);
    camera.SetPerspective(1.0f, DegToRad(fov));

    if (!TryParseBool(value, "enableDOF", true, camera.mDOF.enable))
        return false;

    if (!TryParseFloat(value, "aperture", true, camera.mDOF.aperture))
        return false;

    if (!TryParseFloat(value, "focalPlaneDistance", true, camera.mDOF.focalPlaneDistance))
        return false;

    if (!TryParseTextureName(value, "bokehTexture", textures, camera.mDOF.bokehTexture))
        return false;

    if (camera.mDOF.bokehTexture)
    {
        camera.mDOF.bokehTexture->MakeSamplable();
        camera.mDOF.bokehShape = BokehShape::Texture;
    }

    return true;
}

bool LoadScene(const String& path, Scene& scene, RT::Camera& camera)
{
    FILE* fp = fopen(path.Str(), "rb");
    if (!fp)
    {
        NFE_LOG_ERROR("Failed to open file: %s", path.Str());
        return false;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Document d;
    d.ParseStream<0>(is);
    fclose(fp);

    if (!d.IsObject())
    {
        NFE_LOG_ERROR("Failed to parse scene file '%s': %s", path.Str(), d.GetParseError());
        return false;
    }

    MaterialsMap materialsMap;
    TexturesMap texturesMap;

    if (d.HasMember("textures"))
    {
        const rapidjson::Value& texturesArray = d["textures"];
        if (texturesArray.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < texturesArray.Size(); i++)
            {
                String name;
                const TexturePtr texture = ParseTexture(texturesArray[i], texturesMap, name);
                if (!texture)
                    return false;

                if (materialsMap.Exists(name))
                {
                    NFE_LOG_ERROR("Duplicated texture: '%s'", name.Str());
                    return false;
                }

                texturesMap.Insert(name, texture);
                NFE_LOG_INFO("Created texture: '%s'", name.Str());
            }
        }
        else
        {
            NFE_LOG_ERROR("'textures' is expected to be an array");
            return false;
        }
    }

    if (d.HasMember("materials"))
    {
        const rapidjson::Value& materialsArray = d["materials"];
        if (materialsArray.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < materialsArray.Size(); i++)
            {
                const MaterialPtr material = ParseMaterial(materialsArray[i], texturesMap);
                if (!material)
                    return false;

                if (materialsMap.Exists(material->debugName))
                {
                    NFE_LOG_ERROR("Duplicated material: '%s'", material->debugName.Str());
                    return false;
                }

                materialsMap.Insert(material->debugName, material);
                NFE_LOG_INFO("Created material: '%s'", material->debugName.Str());
            }
        }
        else
        {
            NFE_LOG_ERROR("'materials' is expected to be an array");
            return false;
        }
    }

    if (d.HasMember("objects"))
    {
        const rapidjson::Value& objectsArray = d["objects"];
        if (objectsArray.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < objectsArray.Size(); i++)
            {
                if (!ParseObject(objectsArray[i], scene, materialsMap))
                    return false;
            }
        }
        else
        {
            NFE_LOG_ERROR("'objects' is expected to be an array");
            return false;
        }
    }

    if (d.HasMember("lights"))
    {
        const rapidjson::Value& lightsArray = d["lights"];
        if (lightsArray.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < lightsArray.Size(); i++)
            {
                if (!ParseLight(lightsArray[i], scene, texturesMap))
                    return false;
            }
        }
        else
        {
            NFE_LOG_ERROR("'lights' is expected to be an array");
            return false;
        }
    }

    if (d.HasMember("camera"))
    {
        const rapidjson::Value& cameraObject = d["camera"];
        if (!ParseCamera(cameraObject, texturesMap, camera))
        {
            return false;
        }
    }

    return true;
}

} // namespace helpers
} // namespace NFE