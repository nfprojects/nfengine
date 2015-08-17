#include "PCH.hpp"
#include "Scenes.hpp"
#include "Test.hpp"

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

SceneManager* gScene = nullptr;

void CreateSceneMinecraft();
void CreateSceneSponza();
void CreateScenePerformance();
// void CreateSceneSegments();

std::vector<std::function<void()>> gScenes =
{
    CreateScenePerformance,
    CreateSceneSponza,
    CreateSceneMinecraft,
    // CreateSceneSegments,
};

void CreateSceneMinecraft()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.6f, 0.7f, 0.8f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    gScene->SetEnvironment(&envDesc);

    // SUNLIGHT
    EntityID dirLight = gEntityManager->CreateEntity();
    {
        Orientation orient;
        orient.x = Vector(0.0f, -0.0f, -0.0f, 0.0f);
        orient.z = Vector(-1.5f, -1.0f, 0.5f, 0.0f);
        orient.y = Vector(0.0f, 1.0f, 0.0f, 0.0f);

        TransformComponent transform;
        transform.SetOrientation(orient);
        gEntityManager->AddComponent(dirLight, transform);

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
        gEntityManager->AddComponent(dirLight, light);
        */
    }

    // MINECRAFT
    EntityID map = gEntityManager->CreateEntity();
    {
        TransformComponent transform;
        transform.SetPosition(Vector(0, -70.0f, 0));
        gEntityManager->AddComponent(map, transform);

        MeshComponent mesh;
        mesh.SetMeshResource("minecraft.nfm");
        gEntityManager->AddComponent(map, mesh);
    }
}

void CreateSceneSponza()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.6f, 0.7f, 0.8f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    gScene->SetEnvironment(&envDesc);

    EntityID sponza = gEntityManager->CreateEntity();
    {
        TransformComponent transform;
        gEntityManager->AddComponent(sponza, transform);

        MeshComponent mesh;
        mesh.SetMeshResource("sponza.nfm");
        gEntityManager->AddComponent(sponza, mesh);

        CollisionShape* sponzaShape = ENGINE_GET_COLLISION_SHAPE("sponza_collision_shape.nfcs");
        BodyComponent body;
        body.EnablePhysics(sponzaShape);
        body.SetMass(0.0);
        gEntityManager->AddComponent(sponza, body);
    }

    EntityID lightEntity = gEntityManager->CreateEntity();
    {
        TransformComponent transform;
        transform.SetPosition(Vector(0.0f, 3.5f, 0.0f));
        gEntityManager->AddComponent(lightEntity, transform);

        LightComponent light;
        OmniLightDesc omni;
        omni.shadowFadeStart = 12.0f;
        omni.maxShadowDistance = 120.0f;
        omni.radius = 90.0f;
        light.SetOmniLight(&omni);
        light.SetColor(Float3(50, 50, 50));
        light.SetShadowMap(512);
        gEntityManager->AddComponent(lightEntity, light);
    }
}

/**
 * Performance test (many objects and shadowmaps)
 */
void CreateScenePerformance()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.6f, 0.7f, 0.8f, 0.0f);
    envDesc.backgroundColor = Vector(0.3f, 0.35f, 0.4f, 0.01f);
    gScene->SetEnvironment(&envDesc);

    for (int x = -4; x < 5; x++)
    {
        for (int z = -4; z < 5; z++)
        {
            Vector offset = 12.0f * Vector(static_cast<float>(x),
                                           0.0f,
                                           static_cast<float>(z));

            EntityID chamber = gEntityManager->CreateEntity();
            {
                TransformComponent transform;
                transform.SetPosition(offset);
                gEntityManager->AddComponent(chamber, transform);

                MeshComponent mesh;
                mesh.SetMeshResource("chamber.nfm");
                gEntityManager->AddComponent(chamber, mesh);

                BodyComponent body;
                body.EnablePhysics(ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs"));
                gEntityManager->AddComponent(chamber, body);
            }

            OmniLightDesc omni;
            omni.shadowFadeStart = 80.0f;
            omni.maxShadowDistance = 120.0f;
            omni.radius = 8.0f;
            LightComponent light;
            light.SetOmniLight(&omni);
            light.SetColor(Float3(50, 50, 50));
            light.SetShadowMap(32);


            EntityID mainLight = gEntityManager->CreateEntity();
            {
                TransformComponent transform;
                transform.SetPosition(offset + Vector(0.0f, 3.5f, 0.0f));
                gEntityManager->AddComponent(mainLight, transform);
                gEntityManager->AddComponent(mainLight, light);
            }

            EntityID lightA = gEntityManager->CreateEntity();
            {
                TransformComponent transform;
                transform.SetPosition(offset + Vector(6.0f, 1.8f, 0.0f));
                gEntityManager->AddComponent(lightA, transform);

                omni.radius = 3.0f;
                light.SetOmniLight(&omni);
                light.SetColor(Float3(5.0f, 0.5f, 0.25f));
                gEntityManager->AddComponent(lightA, light);
            }

            EntityID lightB = gEntityManager->CreateEntity();
            {
                TransformComponent transform;
                transform.SetPosition(offset + Vector(0.0f, 1.8f, 6.0f));
                gEntityManager->AddComponent(lightB, transform);

                omni.radius = 3.0f;
                light.SetOmniLight(&omni);
                light.SetColor(Float3(5.0f, 0.5f, 0.25f));
                gEntityManager->AddComponent(lightB, light);
            }

            for (int i = -3; i <= 3; i++)
            {
                for (int j = 0; j < 1; j++)
                {
                    for (int k = -3; k <= 3; k++)
                    {
                        EntityID cube = gEntityManager->CreateEntity();

                        TransformComponent transform;
                        transform.SetPosition(offset + Vector(0.0f, 0.25f, 0.0f) +
                                              0.6f * Vector(static_cast<float>(i),
                                                            static_cast<float>(j),
                                                            static_cast<float>(k)));
                        gEntityManager->AddComponent(cube, transform);

                        MeshComponent mesh;
                        mesh.SetMeshResource("cube.nfm");
                        gEntityManager->AddComponent(cube, mesh);

                        BodyComponent body;
                        body.EnablePhysics(ENGINE_GET_COLLISION_SHAPE("shape_box"));
                        gEntityManager->AddComponent(cube, body);
                    }
                }
            }
        }
    }
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
    envDesc.ambientLight = Vector(0.6f, 0.7f, 0.8f, 0.0f);
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
        light->SetColor(Float3(5.0f, 0.5f, 0.25f));
        entity.Serialize(&segmentDesc, Vector());

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(0.0f, 1.8f, 6.0f));
        light = new LightComponent(&entity);
        omni.radius = 3.0f;
        light->SetOmniLight(&omni);
        light->SetColor(Float3(5.0f, 0.5f, 0.25f));
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

void InitScene(int sceneId)
{
    if (sceneId < 0 || sceneId >= static_cast<int>(gScenes.size()))
        return;

    LOG_INFO("Initializing scene %d", sceneId);
    gScenes[sceneId]();
}
