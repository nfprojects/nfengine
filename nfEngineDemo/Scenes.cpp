#include "PCH.hpp"
#include "Scenes.hpp"
#include "Test.hpp"

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

void CreateSceneSimple(SceneManager* scene);
void CreateSceneMinecraft(SceneManager* scene);
void CreateSceneSponza(SceneManager* scene);
void CreateScenePerformance(SceneManager* scene);
// void CreateSceneSegments(SceneManager* scene);

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
    EntityManager* entityManager = scene->GetEntityManager();

    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.3f, 0.35f, 0.4f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    scene->SetEnvironment(&envDesc);

    // SUNLIGHT
    EntityID dirLight = entityManager->CreateEntity();
    {
        Orientation orient;
        orient.x = Vector(0.0f, -0.0f, -0.0f, 0.0f);
        orient.z = Vector(-1.5f, -1.0f, 0.5f, 0.0f);
        orient.y = Vector(0.0f, 1.0f, 0.0f, 0.0f);

        TransformComponent transform;
        transform.SetOrientation(orient);
        entityManager->AddComponent(dirLight, transform);

        // TODO: directional lights are not yet supported
        /*
        DirLightDesc dirLightDesc;
        dirLightDesc.farDist = 100.0f;
        dirLightDesc.splits = 4;
        dirLightDesc.lightDist = 1000.0f;

        LightComponent light;
        light.SetDirLight(&dirLightDesc);
        light.SetColor(Float3(2.2f, 2.0f, 1.8f));
        light.SetShadowMap(1024);
        entityManager->AddComponent(dirLight, light);
        */
    }

    // MINECRAFT
    EntityID map = entityManager->CreateEntity();
    {
        TransformComponent transform;
        transform.SetPosition(Vector(0.0f, -70.0f, 0.0f));
        entityManager->AddComponent(map, transform);

        MeshComponent mesh;
        mesh.SetMeshResource("minecraft.nfm");
        entityManager->AddComponent(map, mesh);
    }
}

void CreateSceneSponza(SceneManager* scene)
{
    EntityManager* entityManager = scene->GetEntityManager();

    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.3f, 0.35f, 0.4f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    scene->SetEnvironment(&envDesc);

    EntityID sponza = entityManager->CreateEntity();
    {
        TransformComponent transform;
        entityManager->AddComponent(sponza, transform);

        MeshComponent mesh;
        mesh.SetMeshResource("sponza.nfm");
        entityManager->AddComponent(sponza, mesh);

        CollisionShape* sponzaShape = ENGINE_GET_COLLISION_SHAPE("sponza_collision_shape.nfcs");
        BodyComponent body;
        body.EnablePhysics(sponzaShape);
        body.SetMass(0.0);
        entityManager->AddComponent(sponza, body);
    }

    EntityID lightEntity = entityManager->CreateEntity();
    {
        TransformComponent transform;
        transform.SetPosition(Vector(0.0f, 3.5f, 0.0f));
        entityManager->AddComponent(lightEntity, transform);

        LightComponent light;
        OmniLightDesc omni;
        omni.shadowFadeStart = 12.0f;
        omni.maxShadowDistance = 120.0f;
        omni.radius = 90.0f;
        light.SetOmniLight(&omni);
        light.SetColor(Float3(8.0f, 8.0f, 8.0f));
        light.SetShadowMap(512);
        entityManager->AddComponent(lightEntity, light);
    }
}

void CreateChamberArray(SceneManager* scene,
                        int chambersX, int chambersZ,
                        int boxesX, int boxesZ, int boxesY)
{
    EntityManager* entityManager = scene->GetEntityManager();

    // set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.3f, 0.35f, 0.4f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    scene->SetEnvironment(&envDesc);

    for (int x = -chambersX; x <= chambersX; x++)
    {
        for (int z = -chambersZ; z <= chambersZ; z++)
        {
            Vector offset = 12.0f * Vector(static_cast<float>(x),
                                           0.0f,
                                           static_cast<float>(z));

            EntityID chamber = entityManager->CreateEntity();
            {
                TransformComponent transform;
                transform.SetPosition(offset);
                entityManager->AddComponent(chamber, transform);

                MeshComponent mesh;
                mesh.SetMeshResource("chamber.nfm");
                entityManager->AddComponent(chamber, mesh);

                BodyComponent body;
                body.EnablePhysics(ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs"));
                entityManager->AddComponent(chamber, body);
            }

            OmniLightDesc omni;
            omni.shadowFadeStart = 80.0f;
            omni.maxShadowDistance = 120.0f;
            omni.radius = 8.0f;
            LightComponent light;
            light.SetOmniLight(&omni);
            light.SetColor(Float3(12.0f, 12.0f, 12.0f));
            light.SetShadowMap(32);

            EntityID mainLight = entityManager->CreateEntity();
            {
                TransformComponent transform;
                transform.SetPosition(offset + Vector(0.0f, 3.5f, 0.0f));
                entityManager->AddComponent(mainLight, transform);
                entityManager->AddComponent(mainLight, light);
            }

            EntityID lightA = entityManager->CreateEntity();
            {
                TransformComponent transform;
                transform.SetPosition(offset + Vector(6.0f, 1.8f, 0.0f));
                entityManager->AddComponent(lightA, transform);

                omni.radius = 3.0f;
                LightComponent light;
                light.SetOmniLight(&omni);
                light.SetColor(Float3(1.0f, 0.2f, 0.1f));
                light.SetShadowMap(0);
                entityManager->AddComponent(lightA, light);
            }

            EntityID lightB = entityManager->CreateEntity();
            {
                TransformComponent transform;
                transform.SetPosition(offset + Vector(0.0f, 1.8f, 6.0f));
                entityManager->AddComponent(lightB, transform);

                omni.radius = 3.0f;
                LightComponent light;
                light.SetOmniLight(&omni);
                light.SetColor(Float3(1.0f, 0.2f, 0.1f));
                light.SetShadowMap(0);
                entityManager->AddComponent(lightB, light);
            }

            for (int i = -boxesX; i <= boxesX; i++)
            {
                for (int j = 0; j < boxesY; j++)
                {
                    for (int k = -boxesZ; k <= boxesZ; k++)
                    {
                        EntityID cube = entityManager->CreateEntity();

                        TransformComponent transform;
                        transform.SetPosition(offset + Vector(0.0f, 0.25f, 0.0f) +
                                              0.6f * Vector(static_cast<float>(i),
                                                            static_cast<float>(j),
                                                            static_cast<float>(k)));
                        entityManager->AddComponent(cube, transform);

                        MeshComponent mesh;
                        mesh.SetMeshResource("cube.nfm");
                        entityManager->AddComponent(cube, mesh);

                        BodyComponent body;
                        body.EnablePhysics(ENGINE_GET_COLLISION_SHAPE("shape_box"));
                        entityManager->AddComponent(cube, body);
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
    CreateChamberArray(scene, 0, 0, 2, 2, 1);
}

/**
 * Performance test (many objects and shadowmaps)
 */
void CreateScenePerformance(SceneManager* scene)
{
    CreateChamberArray(scene, 2, 2, 2, 2, 2); // LKTODO revert
}

// TODO: restore when scene segments are implemented
/**
 * Infinite looped scene.
 */
/*
void CreateSceneSegments()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.3f, 0.35f, 0.4f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    gScene->SetEnvironment(&envDesc);

    NFE::Common::BufferOutputStream segmentDesc;

    Matrix mat = MatrixRotationNormal(Vector(0, 1, 0), NFE_MATH_PI / 4.0f);

    // create segments description buffer
    {
        OmniLightDesc omni;
        LightComponent* light;
        Entity entity;
        entity.SetPosition(Vector());
        //entity->SetMatrix(mat);
        MeshComponent* mesh = new MeshComponent(&entity);
        mesh->SetMeshResource("chamber.nfm");
        BodyComponent* body = new BodyComponent(&entity);
        body->EnablePhysics(ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs"));
        entity.Serialize(&segmentDesc, Vector());

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(0.0f, 3.5f, 0.0f));

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(6.0f, 1.8f, 0.0f));
        light = new LightComponent(&entity);
        omni.radius = 3.0f;
        light->SetOmniLight(&omni);
        light->SetColor(Float3(1.0f, 0.5f, 0.25f));
        entity.Serialize(&segmentDesc, Vector());

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(0.0f, 1.8f, 6.0f));
        light = new LightComponent(&entity);
        omni.radius = 3.0f;
        light->SetOmniLight(&omni);
        light->SetColor(Float3(1.0f, 0.5f, 0.25f));
        entity.Serialize(&segmentDesc, Vector());
    }

#define SEG_AXIS_NUM 12

    Segment* segments[SEG_AXIS_NUM][SEG_AXIS_NUM];

    // create segments array
    for (int i = 0; i < SEG_AXIS_NUM; i++)
    {
        for (int j = 0; j < SEG_AXIS_NUM; j++)
        {
            char segName[32];
            sprintf_s(segName, "seg_%i_%i", i, j);
            segments[i][j] = gScene->CreateSegment(segName, Vector(5.99f, 1000.0f, 5.99f));
            segments[i][j]->AddEntityFromRawBuffer(segmentDesc.GetData(), segmentDesc.GetSize());
        }
    }

    // create links
    for (int x = 0; x < SEG_AXIS_NUM; x++)
    {
        for (int z = 0; z < SEG_AXIS_NUM; z++)
        {
            //make inifinite loop
            for (int depth = 1; depth <= 5; depth++)
            {
                gScene->CreateLink(segments[x][z], segments[(x + depth) % SEG_AXIS_NUM][z],
                                   Vector(depth * 12.0f, 0.0f, 0.0f));
                gScene->CreateLink(segments[x][z], segments[x][(z + depth) % SEG_AXIS_NUM],
                                   Vector(0.0, 0.0f, depth * 12.0f));
            }
        }
    }

    // Set focus
    gScene->SetFocusSegment(segments[0][0]);
}
*/

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
