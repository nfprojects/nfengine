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
    Entity* pDirLightEnt = gScene->CreateEntity();
    XOrientation orient;
    orient.x = Vector(0.0f, -0.0f, -0.0f, 0.0f);
    orient.z = Vector(-1.5f, -1.0f, 0.5f, 0.0f);
    orient.y = Vector(0.0f, 1.0f, 0.0f, 0.0f);
    pDirLightEnt->SetOrientation(&orient);
    DirLightDesc dirLight;
    dirLight.farDist = 100.0f;
    dirLight.splits = 4;
    dirLight.lightDist = 1000.0f;

    LightComponent* pDirLight = new LightComponent(pDirLightEnt);
    pDirLight->SetDirLight(&dirLight);
    pDirLight->SetColor(Float3(2.2f, 2.0f, 1.8f));
    pDirLight->SetShadowMap(1024);

    // MINECRAFT
    Entity* pEnt = gScene->CreateEntity();
    pEnt->SetPosition(Vector(0, -70.0f, 0));

    MeshComponent* pMesh = new MeshComponent(pEnt);
    pMesh->SetMeshResource("minecraft.nfm");
}

void CreateSceneSponza()
{
    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.02f, 0.03f, 0.04f, 0.0f);
    envDesc.backgroundColor = Vector(0.02f, 0.03f, 0.04f, 0.01f);
    gScene->SetEnvironment(&envDesc);

    Entity* pEnt = gScene->CreateEntity();
    pEnt->SetPosition(Vector(0, 0, 0));

    MeshComponent* pMesh = new MeshComponent(pEnt);
    pMesh->SetMeshResource("sponza.nfm");

    CollisionShape* pSponzaShape = ENGINE_GET_COLLISION_SHAPE("sponza_collision_shape.nfcs");

    BodyComponent* pFloorBody = new BodyComponent(pEnt);
    pFloorBody->EnablePhysics(pSponzaShape);
    pFloorBody->SetMass(0.0);

    pEnt = gScene->CreateEntity();
    pEnt->SetPosition(Vector(0.0f, 3.5f, 0.0f));
    LightComponent* pLight = new LightComponent(pEnt);
    OmniLightDesc omni;
    omni.shadowFadeStart = 12.0f;
    omni.shadowFadeEnd = 120.0f;
    omni.radius = 90.0f;
    pLight->SetOmniLight(&omni);
    pLight->SetColor(Float3(50, 50, 50));
    pLight->SetShadowMap(512);
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

            Entity* pEntity = gScene->CreateEntity();
            pEntity->SetPosition(offset);
            MeshComponent* pMesh = new MeshComponent(pEntity);
            pMesh->SetMeshResource("chamber.nfm");
            BodyComponent* pBody = new BodyComponent(pEntity);
            pBody->EnablePhysics(ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs"));


            LightComponent* pLight;
            OmniLightDesc omni;
            pEntity = gScene->CreateEntity();
            pEntity->SetPosition(offset + Vector(0.0f, 3.5f, 0.0f));
            pLight = new LightComponent(pEntity);

            omni.shadowFadeStart = 80.0f;
            omni.shadowFadeEnd = 120.0f;
            omni.radius = 8.0f;
            pLight->SetOmniLight(&omni);
            pLight->SetColor(Float3(50, 50, 50));
            pLight->SetShadowMap(32);


            pEntity = gScene->CreateEntity();
            pEntity->SetPosition(offset + Vector(6.0f, 1.8f, 0.0f));
            pLight = new LightComponent(pEntity);
            omni.radius = 3.0f;
            pLight->SetOmniLight(&omni);
            pLight->SetColor(Float3(5.0f, 0.5f, 0.25f));

            pEntity = gScene->CreateEntity();
            pEntity->SetPosition(offset + Vector(0.0f, 1.8f, 6.0f));
            pLight = new LightComponent(pEntity);
            omni.radius = 3.0f;
            pLight->SetOmniLight(&omni);
            pLight->SetColor(Float3(5.0f, 0.5f, 0.25f));

            for (int i = -3; i <= 3; i++)
            {
                for (int j = 0; j < 1; j++)
                {
                    for (int k = -3; k <= 3; k++)
                    {
                        Entity* pCube = gScene->CreateEntity();
                        pCube->SetPosition(offset + 0.6f * Vector(static_cast<float>(i),
                                                                  static_cast<float>(j),
                                                                  static_cast<float>(k)) +
                                           Vector(0.0f, 0.25f, 0.0f));

                        MeshComponent* pMesh = new MeshComponent(pCube);
                        pMesh->SetMeshResource("cube.nfm");

                        BodyComponent* pBody = new BodyComponent(pCube);
                        pBody->SetMass(0.0f);
                        pBody->EnablePhysics(ENGINE_GET_COLLISION_SHAPE("shape_box"));
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
        LightComponent* pLight;
        Entity entity;
        entity.SetPosition(Vector());
        //pEntity->SetMatrix(mat);
        MeshComponent* pMesh = new MeshComponent(&entity);
        pMesh->SetMeshResource("chamber.nfm");
        BodyComponent* pBody = new BodyComponent(&entity);
        pBody->EnablePhysics(ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs"));
        entity.Serialize(&segmentDesc, Vector());

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(0.0f, 3.5f, 0.0f));

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(6.0f, 1.8f, 0.0f));
        pLight = new LightComponent(&entity);
        omni.radius = 3.0f;
        pLight->SetOmniLight(&omni);
        pLight->SetColor(Float3(5.0f, 0.5f, 0.25f));
        entity.Serialize(&segmentDesc, Vector());

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(0.0f, 1.8f, 6.0f));
        pLight = new LightComponent(&entity);
        omni.radius = 3.0f;
        pLight->SetOmniLight(&omni);
        pLight->SetColor(Float3(5.0f, 0.5f, 0.25f));
        entity.Serialize(&segmentDesc, Vector());
    }

#define SEG_AXIS_NUM 12

    Segment* pSegments[SEG_AXIS_NUM][SEG_AXIS_NUM];

    // create segments array
    for (int i = 0; i < SEG_AXIS_NUM; i++)
    {
        for (int j = 0; j < SEG_AXIS_NUM; j++)
        {
            char segName[32];
            sprintf_s(segName, "seg_%i_%i", i, j);
            pSegments[i][j] = gScene->CreateSegment(segName, Vector(5.99f, 1000.0f, 5.99f));
            pSegments[i][j]->AddEntityFromRawBuffer(segmentDesc.GetData(), segmentDesc.GetSize());
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
                gScene->CreateLink(pSegments[x][z], pSegments[(x + depth) % SEG_AXIS_NUM][z],
                                   Vector(depth * 12.0f, 0.0f, 0.0f));
                gScene->CreateLink(pSegments[x][z], pSegments[x][(z + depth) % SEG_AXIS_NUM],
                                   Vector(0.0, 0.0f, depth * 12.0f));
            }
        }
    }

    // Set focus
    gScene->SetFocusSegment(pSegments[0][0]);
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
