#include "stdafx.hpp"
#include "Scenes.hpp"

#include "../nfEngine/nfCore/Engine.hpp"
#include "../nfEngine/nfCore/SceneEvent.hpp"
#include "../nfEngine/nfCore/SceneSegment.hpp"

using namespace NFE;
using namespace NFE::Render;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

SceneManager* gScene = nullptr;

void CreateSceneMinecraft();
void CreateSceneSponza();
void CreateScenePerformance();
void CreateSceneSegments();

std::vector<std::function<void()>> gScenes =
{
    CreateScenePerformance,
    CreateSceneSegments,
    CreateSceneSponza,
    CreateSceneMinecraft,
};

void CreateSceneMinecraft()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.02f, 0.03f, 0.04f, 0.0f);
    envDesc.backgroundColor = Vector(0.02f, 0.03f, 0.04f, 0.01f);
    gScene->SetEnvironment(&envDesc);

    // SUNLIGHT
    Entity* dirLightEnt = gScene->CreateEntity();
    XOrientation orient;
    orient.x = Vector(0.0f, -0.0f, -0.0f, 0.0f);
    orient.z = Vector(-1.5f, -1.0f, 0.5f, 0.0f);
    orient.y = Vector(0.0f, 1.0f, 0.0f, 0.0f);
    dirLightEnt->SetOrientation(&orient);
    DirLightDesc dirLightDesc;
    dirLightDesc.farDist = 100.0f;
    dirLightDesc.splits = 4;
    dirLightDesc.lightDist = 1000.0f;

    LightComponent* dirLight = new LightComponent(dirLightEnt);
    dirLight->SetDirLight(&dirLightDesc);
    dirLight->SetColor(Float3(2.2f, 2.0f, 1.8f));
    dirLight->SetShadowMap(1024);

    // MINECRAFT
    Entity* ent = gScene->CreateEntity();
    ent->SetPosition(Vector(0, -70.0f, 0));

    MeshComponent* mesh = new MeshComponent(ent);
    mesh->SetMeshResource("minecraft.nfm");
}

void CreateSceneSponza()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.02f, 0.03f, 0.04f, 0.0f);
    envDesc.backgroundColor = Vector(0.02f, 0.03f, 0.04f, 0.01f);
    gScene->SetEnvironment(&envDesc);

    Entity* ent = gScene->CreateEntity();
    ent->SetPosition(Vector(0, 0, 0));

    MeshComponent* mesh = new MeshComponent(ent);
    mesh->SetMeshResource("sponza.nfm");

    CollisionShape* sponzaShape = ENGINE_GET_COLLISION_SHAPE("sponza_collision_shape.nfcs");

    BodyComponent* floorBody = new BodyComponent(ent);
    floorBody->EnablePhysics(sponzaShape);
    floorBody->SetMass(0.0);

    ent = gScene->CreateEntity();
    ent->SetPosition(Vector(0.0f, 3.5f, 0.0f));
    LightComponent* light = new LightComponent(ent);
    OmniLightDesc omni;
    omni.shadowFadeStart = 12.0f;
    omni.shadowFadeEnd = 120.0f;
    omni.radius = 90.0f;
    light->SetOmniLight(&omni);
    light->SetColor(Float3(50, 50, 50));
    light->SetShadowMap(512);
}

/**
 * Performance test (many objects and shadowmaps)
 */
void CreateScenePerformance()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.02f, 0.03f, 0.04f, 0.0f);
    envDesc.backgroundColor = Vector(0.02f, 0.03f, 0.04f, 0.01f);
    gScene->SetEnvironment(&envDesc);

    for (int x = -4; x < 5; x++)
    {
        for (int z = -4; z < 5; z++)
        {
            Vector offset = 12.0f * Vector(static_cast<float>(x),
                                           0.0f,
                                           static_cast<float>(z));

            Entity* entity = gScene->CreateEntity();
            entity->SetPosition(offset);
            MeshComponent* mesh = new MeshComponent(entity);
            mesh->SetMeshResource("chamber.nfm");
            BodyComponent* body = new BodyComponent(entity);
            body->EnablePhysics(ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs"));


            LightComponent* light;
            OmniLightDesc omni;
            entity = gScene->CreateEntity();
            entity->SetPosition(offset + Vector(0.0f, 3.5f, 0.0f));
            light = new LightComponent(entity);

            omni.shadowFadeStart = 80.0f;
            omni.shadowFadeEnd = 120.0f;
            omni.radius = 8.0f;
            light->SetOmniLight(&omni);
            light->SetColor(Float3(50, 50, 50));
            light->SetShadowMap(32);


            entity = gScene->CreateEntity();
            entity->SetPosition(offset + Vector(6.0f, 1.8f, 0.0f));
            light = new LightComponent(entity);
            omni.radius = 3.0f;
            light->SetOmniLight(&omni);
            light->SetColor(Float3(5.0f, 0.5f, 0.25f));

            entity = gScene->CreateEntity();
            entity->SetPosition(offset + Vector(0.0f, 1.8f, 6.0f));
            light = new LightComponent(entity);
            omni.radius = 3.0f;
            light->SetOmniLight(&omni);
            light->SetColor(Float3(5.0f, 0.5f, 0.25f));

            for (int i = -3; i <= 3; i++)
            {
                for (int j = 0; j < 1; j++)
                {
                    for (int k = -3; k <= 3; k++)
                    {
                        Entity* cube = gScene->CreateEntity();
                        cube->SetPosition(offset + 0.6f * Vector(static_cast<float>(i),
                                                                 static_cast<float>(j),
                                                                 static_cast<float>(k)) +
                                          Vector(0.0f, 0.25f, 0.0f));

                        MeshComponent* mesh = new MeshComponent(cube);
                        mesh->SetMeshResource("cube.nfm");

                        BodyComponent* body = new BodyComponent(cube);
                        body->SetMass(0.0f);
                        body->EnablePhysics(ENGINE_GET_COLLISION_SHAPE("shape_box"));
                    }
                }
            }
        }
    }
}

/**
 * Infinite looped scene.
 */
void CreateSceneSegments()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.02f, 0.03f, 0.04f, 0.0f);
    envDesc.backgroundColor = Vector(0.02f, 0.03f, 0.04f, 0.01f);
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

int GetScenesNum()
{
    return static_cast<int>(gScenes.size());
}

void InitScene(int sceneId)
{
    if (sceneId < 0 || sceneId >= static_cast<int>(gScenes.size()))
        return;

    gScenes[sceneId]();
}
