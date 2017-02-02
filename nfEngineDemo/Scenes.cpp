#include "PCH.hpp"
#include "Scenes.hpp"
#include "Main.hpp"

#include "nfCore/Renderer/RenderScene.hpp" // TODO remove
#include "nfCore/Scene/Systems/RendererSystem.hpp"
#include "nfCore/Scene/Components/ComponentBody.hpp"
#include "nfCore/Scene/Components/ComponentLight.hpp"
#include "nfCore/Scene/Components/ComponentMesh.hpp"


namespace NFE {

using namespace Common;
using namespace Renderer;
using namespace Math;
using namespace Scene;
using namespace Resource;

void CreateSceneSimple(SceneManager* scene);
void CreateSceneMinecraft(SceneManager* scene);
void CreateSceneSponza(SceneManager* scene);
void CreateScenePerformance(SceneManager* scene);

std::vector<std::function<void(SceneManager*)>> gScenes =
{
    CreateSceneSimple,
    CreateScenePerformance,
    CreateSceneSponza,
    CreateSceneMinecraft,
    // CreateSceneSegments,
};

void CreateSceneMinecraft(SceneManager* scene)
{
    EntitySystem* entitySystem = scene->GetSystem<EntitySystem>();

    //set ambient & background color
    EnvironmentDesc envDesc;
    envDesc.ambientLight = Vector(0.3f, 0.35f, 0.4f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    scene->GetSystem<RendererSystem>()->GetRenderScene()->SetEnvironment(envDesc);

    // TODO directional light

    // MINECRAFT
    Entity* map = entitySystem->CreateEntity();
    map->SetGlobalPosition(Vector(0.0f, -70.0f, 0.0f));

    UniquePtr<MeshComponent> meshComponent(new MeshComponent);
    meshComponent->SetMeshResource("minecraft.nfm");
    map->AddComponent(std::move(meshComponent));
}

void CreateSceneSponza(SceneManager* scene)
{
    EntitySystem* entitySystem = scene->GetSystem<EntitySystem>();

    //set ambient & background color
    EnvironmentDesc envDesc;
    envDesc.ambientLight = Vector(0.3f, 0.35f, 0.4f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    scene->GetSystem<RendererSystem>()->GetRenderScene()->SetEnvironment(envDesc);

    Entity* sponza = entitySystem->CreateEntity();
    {
        UniquePtr<MeshComponent> mesh(new MeshComponent);
        mesh->SetMeshResource("sponza.nfm");
        sponza->AddComponent(std::move(mesh));

        UniquePtr<BodyComponent> body(new BodyComponent);
        CollisionShape* sponzaShape = ENGINE_GET_COLLISION_SHAPE("sponza_collision_shape.nfcs");
        body->SetCollisionShape(sponzaShape);
        body->SetMass(0.0f);
        sponza->AddComponent(std::move(body));
    }

    Entity* lightEntity = entitySystem->CreateEntity();
    lightEntity->SetGlobalPosition(Vector(0.0f, 3.5f, 0.0f));
    {
        OmniLightDesc omni;
        omni.shadowFadeStart = 12.0f;
        omni.maxShadowDistance = 120.0f;
        omni.radius = 90.0f;

        UniquePtr<LightComponent> light(new LightComponent);
        light->SetOmniLight(omni);
        light->SetColor(Float3(8.0f, 8.0f, 8.0f));
        light->SetShadowMap(512);
        lightEntity->AddComponent(std::move(light));
    }
}

void CreateChamberArray(SceneManager* scene,
                        int chambersX, int chambersZ,
                        int boxesX, int boxesZ, int boxesY)
{
    EntitySystem* entitySystem = scene->GetSystem<EntitySystem>();

    // set ambient & background color
    EnvironmentDesc envDesc;
    envDesc.ambientLight = Vector(0.3f, 0.35f, 0.4f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    scene->GetSystem<RendererSystem>()->GetRenderScene()->SetEnvironment(envDesc);

    // TODO all of this must be loaded from game object resource

    for (int x = -chambersX; x <= chambersX; x++)
    {
        for (int z = -chambersZ; z <= chambersZ; z++)
        {
            Vector offset = 12.0f * Vector(static_cast<float>(x),
                                           0.0f,
                                           static_cast<float>(z));

            Entity* chamberEntity = entitySystem->CreateEntity();
            chamberEntity->SetGlobalPosition(offset);
            {
                UniquePtr<MeshComponent> mesh(new MeshComponent);
                mesh->SetMeshResource("chamber.nfm");
                chamberEntity->AddComponent(std::move(mesh));

                UniquePtr<BodyComponent> body(new BodyComponent);
                body->SetCollisionShape(ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs"));
                chamberEntity->AddComponent(std::move(body));
            }

            OmniLightDesc omni;
            omni.shadowFadeStart = 80.0f;
            omni.maxShadowDistance = 120.0f;
            omni.radius = 12.0f;

            Entity* mainLightEntity = entitySystem->CreateEntity();
            mainLightEntity->SetGlobalPosition(offset + Vector(0.0f, 3.5f, 0.0f));
            {
                UniquePtr<LightComponent> light(new LightComponent);
                light->SetOmniLight(omni);
                light->SetColor(Float3(20.0f, 20.0f, 20.0f));
                light->SetShadowMap(512);

                mainLightEntity->AddComponent(std::move(light));
            }

            Entity* lightEntityA = entitySystem->CreateEntity();
            lightEntityA->SetGlobalPosition(offset + Vector(6.0f, 1.8f, 0.0f));
            {
                UniquePtr<LightComponent> light(new LightComponent);
                omni.radius = 3.0f;
                light->SetOmniLight(omni);
                light->SetColor(Float3(1.0f, 0.2f, 0.1f));

                lightEntityA->AddComponent(std::move(light));
            }

            Entity* lightEntityB = entitySystem->CreateEntity();
            lightEntityB->SetGlobalPosition(offset + Vector(0.0f, 1.8f, 6.0f));
            {
                UniquePtr<LightComponent> light(new LightComponent);
                omni.radius = 3.0f;
                light->SetOmniLight(omni);
                light->SetColor(Float3(1.0f, 0.2f, 0.1f));

                lightEntityB->AddComponent(std::move(light));
            }

            for (int i = -boxesX; i <= boxesX; i++)
            {
                for (int j = 0; j < boxesY; j++)
                {
                    for (int k = -boxesZ; k <= boxesZ; k++)
                    {
                        Entity* boxEntity = entitySystem->CreateEntity();
                        boxEntity->SetGlobalPosition(offset + Vector(0.0f, 0.25f, 0.0f) +
                                                     0.6f * Vector(static_cast<float>(i),
                                                                   static_cast<float>(j),
                                                                   static_cast<float>(k)));

                        UniquePtr<MeshComponent> mesh(new MeshComponent);
                        mesh->SetMeshResource("cube.nfm");
                        boxEntity->AddComponent(std::move(mesh));

                        UniquePtr<BodyComponent> body(new BodyComponent);
                        body->SetCollisionShape(ENGINE_GET_COLLISION_SHAPE("shape_box"));
                        boxEntity->AddComponent(std::move(body));
                    }
                }
            }
        }
    }
}

/**
 * Performance test (many objects and shadowmaps)
 */
void CreateSceneSimple(SceneManager* scene)
{
    CreateChamberArray(scene, 1, 1, 2, 2, 1);
}

/**
 * Performance test (many objects and shadowmaps)
 */
void CreateScenePerformance(SceneManager* scene)
{
    CreateChamberArray(scene, 4, 4, 4, 4, 2);
}

int GetScenesNum()
{
    return static_cast<int>(gScenes.size());
}

SceneManager* InitScene(int sceneId)
{
    if (sceneId < 0 || sceneId >= static_cast<int>(gScenes.size()))
        return nullptr;

    LOG_INFO("Initializing scene %d", sceneId);

    SceneManager* scene = gEngine->CreateScene();
    gScenes[sceneId](scene);
    return scene;
}

} // namespace NFE
