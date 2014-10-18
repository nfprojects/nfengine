#include "stdafx.hpp"
#include "test.hpp"
#include "Editor.hpp"
#include "../nfEngine/nfCommon/Logger.hpp"

using namespace NFE;
using namespace NFE::Render;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

class CustomWindow;

Random g_Random;
SceneManager* g_pScene = 0;
float g_DeltaTime = 0.0f;

const char* g_pCamTextureName = "/cam_texture";
View* g_pSecondaryCameraView = nullptr;

// enable / disable rendering secondary camera view
//#define DEMO_SEC_CAMERA_ON

#ifdef WIN64
#define PLATFORM_STR "x64"
#else
#define PLATFORM_STR "x86"
#endif


MainCameraView::MainCameraView()
{

}

void MainCameraView::OnPostRender(IRenderContext* pCtx, IGuiRenderer* pGuiRenderer)
{
    const int x = 32;
    const int y = 16;

    if (g_pSecondaryCameraView)
    {
        pGuiRenderer->DrawQuad(pCtx, Recti(x - 1, y - 1, x + 257, y + 257), NULL, NULL, 0x7F000000);
        Texture* pTex = ENGINE_GET_TEXTURE(g_pCamTextureName);
        pGuiRenderer->DrawQuad(pCtx, Recti(x, y, x + 256, y + 256), pTex->GetRendererTexture(), NULL,
                               0xFFFFFFFF);
    }
}

// overload own callback functions
class CustomWindow : public Common::Window
{
public:
    Quaternion cameraOrientation;
    Entity* cameraEntity;
    Camera* camera;
    View* view;

    bool cameraControl;
    float cameraXZ;
    float cameraY;

    void* operator new(size_t size)
    {
        return _aligned_malloc(size, 16);
    }

    void operator delete(void* ptr)
    {
        _aligned_free(ptr);
    }

    CustomWindow()
    {
        cameraEntity = 0;
        camera = 0;
        view = 0;

        cameraControl = 0;
        cameraXZ = 0.0f;
        cameraY = 0.0f;

        cameraOrientation = QuaternionMultiply(QuaternionRotationY(cameraXZ),
                                               QuaternionRotationX(-cameraY));
    }

    ~CustomWindow()
    {
        delete view;
        // TODO: releasing engine before destruction of View without memory leak !!!
    }

    // create camera entity, etc.
    void InitCamera()
    {
        cameraEntity = g_pScene->CreateEntity();
        cameraEntity->SetPosition(Vector(0.0f, 1.6f, -2.0f, 0.0f));


        UpdateCamera();
        Perspective perspective;
        perspective.FoV = NFE_MATH_PI * 60.0f / 180.0f;
        perspective.nearDist = 0.01f;
        perspective.farDist = 1000.0f;
        perspective.aspectRatio = getAspectRatio();

        camera = new Camera(cameraEntity);
        camera->SetPerspective(&perspective);
        g_pScene->SetDefaultCamera(camera);

        BodyComponent* pCameraBody = new BodyComponent(cameraEntity);

        view = new MainCameraView;
        view->SetCamera(camera);
        view->SetWindow(this);
    }

#define CAMERA_ROTATION_SMOOTHING 0.05f
#define CAMERA_TRANSLATION_SMOOTHING 0.2f

    // update camera position and orientation
    void UpdateCamera()
    {
        if (!cameraEntity)
            return;

        Quaternion destOrientation = QuaternionMultiply(QuaternionRotationY(cameraXZ),
                                     QuaternionRotationX(-cameraY));
        destOrientation = QuaternionNormalize(destOrientation);

        //LPF
        Quaternion prevOrientation = cameraOrientation;
        cameraOrientation = QuaternionInterpolate(cameraOrientation, destOrientation,
                            g_DeltaTime / (CAMERA_ROTATION_SMOOTHING + g_DeltaTime));
        cameraOrientation = QuaternionNormalize(cameraOrientation);

        Quaternion rotation = QuaternionMultiply(prevOrientation, QuaternionInverse(cameraOrientation));
        cameraEntity->SetAngularVelocity(-QuaternionToAxis(rotation) / g_DeltaTime);

        Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(cameraOrientation));
        XOrientation orient;
        orient.x = rotMatrix.r[0];
        orient.y = rotMatrix.r[1];
        orient.z = rotMatrix.r[2];
        cameraEntity->SetOrientation(&orient);


        Vector destVelocity = Vector();
        if (cameraControl)
        {
            if (isKeyPressed('W')) destVelocity += orient.z;
            if (isKeyPressed('S')) destVelocity -= orient.z;
            if (isKeyPressed('D')) destVelocity += orient.x;
            if (isKeyPressed('A')) destVelocity -= orient.x;
            if (isKeyPressed('R')) destVelocity += orient.y;
            if (isKeyPressed('F')) destVelocity -= orient.y;

            if (isKeyPressed(VK_SHIFT)) destVelocity *= 30.0f;
            else if (isKeyPressed(VK_CONTROL)) destVelocity *= 0.2f;
            else destVelocity *= 3.0f;
        }

        Vector prevVelocity = cameraEntity->GetVelocity();

        //low pass filter - for smooth camera movement
        float factor = g_DeltaTime / (CAMERA_TRANSLATION_SMOOTHING + g_DeltaTime);
        cameraEntity->SetVelocity(VectorLerp(prevVelocity, destVelocity, factor));
    }

    void OnKeyPress(int key)
    {
        if (key == VK_F1)
        {
            BOOL fullscreen = getFullscreenMode();
            setFullscreenMode(!fullscreen);
        }

        XOrientation orient;

        //place spot light
        if (key == 'T')
        {
            Entity* pLightEntity = g_pScene->CreateEntity();
            cameraEntity->GetOrientation(&orient);
            pLightEntity->SetOrientation(&orient);
            pLightEntity->SetPosition(cameraEntity->GetPosition());

            LightComponent* pLight = new LightComponent(pLightEntity);
            SpotLightDesc lightDesc;
            lightDesc.nearDist = 0.1f;
            lightDesc.farDist = 500.0f;
            lightDesc.cutoff = NFE_MATH_PI / 4.0f;
            lightDesc.maxShadowDistance = 60.0;
            pLight->SetSpotLight(&lightDesc);
            pLight->SetColor(Float3(600, 200, 50));
            pLight->SetLightMap("flashlight.jpg");
            pLight->SetShadowMap(1024);

            g_pSelectedEntity = pLightEntity;
        }

        //place omni light
        if (key == 'O')
        {
            OmniLightDesc lightDesc;
            lightDesc.radius = 10.0f;
            lightDesc.shadowFadeStart = 20.0;
            lightDesc.shadowFadeEnd = 30.0;

            Entity* pLightEntity = g_pScene->CreateEntity();
            pLightEntity->SetPosition(cameraEntity->GetPosition());

            LightComponent* pLight = new LightComponent(pLightEntity);
            pLight->SetOmniLight(&lightDesc);
            pLight->SetColor(Float3(600, 600, 600));
            pLight->SetShadowMap(512);

            g_pSelectedEntity = pLightEntity;
        }
    }

    void OnMouseDown(UINT button, int x, int y)
    {
        XOrientation camOrient;
        cameraEntity->GetOrientation(&camOrient);

        if (button == 0)
            cameraControl = true;

        //shoot a cube
        else if (button == 1)
        {
            Entity* pCube = g_pScene->CreateEntity();
            g_pSelectedEntity = pCube;
            pCube->SetPosition(cameraEntity->GetPosition() + camOrient.z);
            pCube->SetVelocity(0.1f * camOrient.z);
            //pCube->SetMaxLifeTime(5.0f);

            MeshComponent* pMesh = new MeshComponent(pCube);
            pMesh->SetMeshResource("cube.nfm");

            BodyComponent* pBody = new BodyComponent(pCube);
            pBody->SetMass(10.0f);
            pBody->EnablePhysics((CollisionShape*)EngineGetResource(ResourceType::COLLISION_SHAPE,
                                 "shape_box"));


            OmniLightDesc lightDesc;
            lightDesc.radius = 4.0f;
            lightDesc.shadowFadeStart = 20.0;
            lightDesc.shadowFadeEnd = 30.0;

            Entity* pLightEntity = g_pScene->CreateEntity();
            pCube->Attach(pLightEntity);
            //pLightEntity->SetLocalPosition(Vector(0.0f, 1.0f, 0.0f));

            LightComponent* pLight = new LightComponent(pLightEntity);
            pLight->SetOmniLight(&lightDesc);
            pLight->SetColor(Float3(1.0f, 1.0f, 10.0f));
            pLight->SetShadowMap(0);

        }
        else
        {
            Entity* pBarrel = g_pScene->CreateEntity();
            g_pSelectedEntity = pBarrel;
            pBarrel->SetPosition(cameraEntity->GetPosition() + camOrient.z);
            pBarrel->SetVelocity(30.0f * camOrient.z);

            MeshComponent* pMesh = new MeshComponent(pBarrel);
            pMesh->SetMeshResource("barrel.nfm");

            BodyComponent* pBody = new BodyComponent(pBarrel);
            pBody->SetMass(20.0f);
            pBody->EnablePhysics((CollisionShape*)EngineGetResource(ResourceType::COLLISION_SHAPE,
                                 "shape_barrel"));

        }
    }

    void OnMouseMove(int x, int y, int deltaX, int deltaY)
    {
        if (cameraControl)
        {
            float fDeltaX = (float)deltaX * 0.005f;
            float fDeltaY = (float)deltaY * 0.005f;

            cameraXZ += fDeltaX;
            cameraY -= fDeltaY;
            cameraXZ = fmodf(cameraXZ, 2.0f * NFE_MATH_PI);

            if (cameraY > NFE_MATH_PI / 2.0f) cameraY = NFE_MATH_PI / 2.0f;
            if (cameraY < -NFE_MATH_PI / 2.0f) cameraY = -NFE_MATH_PI / 2.0f;
        }
    }

    void OnMouseUp(UINT button)
    {
        if (button == 0)
            cameraControl = false;
    }

    // window resized
    void OnResize(UINT width, UINT height)
    {
        if (camera)
        {
            if (width || height)
            {
                Perspective perspective;
                camera->GetPerspective(&perspective);
                perspective.aspectRatio = (float)width / (float)height;
                camera->SetPerspective(&perspective);
            }
        }
    }
};

void ProcessSceneEvents()
{
    SceneEvent eventID;
    void* pData;
    while (g_pScene->GetEventSystem()->Pop(&eventID, &pData))
    {
        switch (eventID)
        {
            case SceneEvent::EntityDestroy:
            {
                EventEntityDestroy* pEvent = (EventEntityDestroy*)pData;
                LOG_INFO("Entity destroyed: %p", pEvent->entity);
                if (pEvent->entity == g_pSelectedEntity)
                    g_pSelectedEntity = nullptr;
                break;
            }

            case SceneEvent::BodyCollide:
            {
                EventBodyCollide* pEvent = (EventBodyCollide*)pData;
                /*
                if (pEvent->pBodyA->GetMass() > NFE_MATH_EPSILON)
                    g_pScene->EnqueueDeleteEntity(pEvent->pBodyA->GetOwner());
                else
                    g_pScene->EnqueueDeleteEntity(pEvent->pBodyB->GetOwner());
                */
                break;
            }
        }
    }
}

void BVHTest()
{
    Common::Timer timer;

    Random rand = Random(1);

#define NUM_INSERTIONS 100000
    Box* pBoxes = (Box*)_aligned_malloc(NUM_INSERTIONS * sizeof(Box), 16);
    for (int i = 0; i < NUM_INSERTIONS; i++)
    {
        Vector pos = rand.GetFloat3();
        pos *= 100.0f;
        pBoxes[i] = Box(pos - Vector(0.5f, 0.5f, 0.5), pos + Vector(0.5f, 0.5f, 0.5));
    }


    Util::BVH bvh;
    timer.Start();
    for (int i = 0; i < NUM_INSERTIONS; i++)
    {
        bvh.Insert(pBoxes[i], (void*)i);
    }
    double buildTime = timer.Stop();

    Box testBox = Box(Vector(50, 50, 50), Vector(60, 60, 60));

    //__asm int 3;
}


// temporary
bool OnLoadCustomShapeResource(ResourceBase* pRes, void* pData)
{
    CollisionShape* pShape = (CollisionShape*)pRes;

    if (strcmp(pShape->GetName(), "shape_floor") == 0)
    {
        pShape->AddBox(Vector(100.0f, 10.0f, 100.0f), MatrixIdentity());
    }
    else if (strcmp(pShape->GetName(), "shape_box") == 0)
    {
        pShape->AddBox(Vector(0.25f, 0.25f, 0.25f), MatrixIdentity());
    }
    else if (strcmp(pShape->GetName(), "shape_frame") == 0)
    {
        // Z axis
        pShape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(-0.475f, -0.475f, 0.0f)));
        pShape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(-0.475f, 0.475f, 0.0f)));
        pShape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(0.475f, -0.475f, 0.0f)));
        pShape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(0.475f, 0.475f, 0.0f)));

        // Y axis
        pShape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(-0.475f, 0.0f, -0.475f)));
        pShape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(-0.475f, 0.0f, 0.475f)));
        pShape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(0.475f, 0.0f, -0.475f)));
        pShape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(0.475f, 0.0f, 0.475f)));

        // X axis
        pShape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, -0.475f, -0.475f)));
        pShape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, -0.475f, 0.475f)));
        pShape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, 0.475f, -0.475f)));
        pShape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, 0.475f, 0.475f)));
    }
    else if (strcmp(pShape->GetName(), "shape_barrel") == 0)
    {
        pShape->AddCylinder(1.31f, 0.421f);
    }

    return true;
}

/*
    List of scenes:
    - SCENE_SPONZA
    - SCENE_MINECRAFT
    - SCENE_SEGMENTS
    - SCENE_SEGMENTS_PERF_TEST
*/
#define SCENE_SEGMENTS_PERF_TEST

void InitSecondaryCamera()
{
    Entity* pCameraEntity = g_pScene->CreateEntity();
    pCameraEntity->SetPosition(Vector(0.0f, 1.6f, 0.0f, 0.0f));

    Perspective perspective;
    perspective.FoV = NFE_MATH_PI * 80.0f / 180.0f;
    perspective.nearDist = 0.01f;
    perspective.farDist = 20.0f;
    perspective.aspectRatio = 1.0f;

    Camera* pCamera = new Camera(pCameraEntity);
    pCamera->SetPerspective(&perspective);
    g_pScene->SetDefaultCamera(pCamera);

    // TODO: fix memory leaks
    g_pSecondaryCameraView = new View;
    g_pSecondaryCameraView->SetCamera(pCamera);
    g_pSecondaryCameraView->SetOffScreen(256, 256, g_pCamTextureName);
}

// temporary
void SetUpCurrentDirectory()
{
    char szFileName[MAX_PATH];
    GetModuleFileNameA(NULL, szFileName, MAX_PATH);
    std::string rootDir = std::string(szFileName);
    size_t found = rootDir.find_last_of("/\\");
    rootDir = rootDir.substr(0, found) + "/../../../";
    SetCurrentDirectoryA(rootDir.c_str());
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    SetUpCurrentDirectory();

    //create window
    CustomWindow* pWindow = new CustomWindow;
    pWindow->setSize(800, 600);
    pWindow->setTitle(L"NFEngine Demo - Initializing engine...");
    pWindow->open();

    //initialize engine
    if (EngineInit() != Result::OK)
        return 1;

    Demo_InitEditorBar();

    //create scene and camera
    g_pScene = EngineCreateScene();

    // --------------------------------
    // Build scene
    // --------------------------------

#ifdef DEMO_SEC_CAMERA_ON
    InitSecondaryCamera();
#endif

    //set ambient & background color
    EnviromentDesc envDesc;
    envDesc.ambientLight = Vector(0.02f, 0.03f, 0.04f, 0.0f);
    //envDesc.m_BackgroundColor = Vector(0.04f, 0.05f, 0.07f, 0.03f);
    envDesc.backgroundColor = Vector(0.02f, 0.03f, 0.04f, 0.01f);
    g_pScene->SetEnvironment(&envDesc);

    CollisionShape* pFloorShape = ENGINE_GET_COLLISION_SHAPE("shape_floor");
    pFloorShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    pFloorShape->Load();
    pFloorShape->AddRef();

    CollisionShape* pFrameShape = ENGINE_GET_COLLISION_SHAPE("shape_frame");
    pFrameShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    pFrameShape->Load();
    pFrameShape->AddRef();

    CollisionShape* pBoxShape = ENGINE_GET_COLLISION_SHAPE("shape_box");
    pBoxShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    pBoxShape->Load();
    pBoxShape->AddRef();

    CollisionShape* pBarrelShape = ENGINE_GET_COLLISION_SHAPE("shape_barrel");
    pBarrelShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    pBarrelShape->Load();
    pBarrelShape->AddRef();

    CollisionShape* pChamberShape = ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs");
    pChamberShape->Load();
    pChamberShape->AddRef();


    pWindow->InitCamera();
    pWindow->setTitle(L"NFEngine Demo");


#ifdef SCENE_MINECRAFT
    // SUNLIGHT
    Entity* pDirLightEnt = g_pScene->CreateEntity();
    XOrientation orient;
    orient.x = Vector(0.0f, -0.0f, -0.0f, 0.0f);
    orient.z = Vector(-1.5f, -1.0f, 0.5f, 0.0f);
    orient.y = Vector(0.0f, 1.0f, 0.0f, 0.0f);
    pDirLightEnt->SetOrientation(&orient);
    DirLightDesc dirLight;
    dirLight.m_Far = 100.0f;
    dirLight.m_Splits = 4;
    dirLight.m_LightDist = 1000.0f;

    LightComponent* pDirLight = new LightComponent(pDirLightEnt);
    pDirLight->SetDirLight(&dirLight);
    pDirLight->SetColor(Float3(2.2, 2, 1.8));
    pDirLight->SetShadowMap(1024);

    // MINECRAFT
    Entity* pEnt = g_pScene->CreateEntity();
    pEnt->SetPosition(Vector(0, -70.0f, 0));

    MeshComponent* pMesh = new MeshComponent(pEnt);
    pMesh->SetMeshResource("minecraft.nfm");
#endif

#ifdef SCENE_SPONZA
    // SPONZA
    Entity* pEnt = g_pScene->CreateEntity();
    pEnt->SetPosition(Vector(0, 0, 0));

    MeshComponent* pMesh = new MeshComponent(pEnt);
    pMesh->SetMeshResource("sponza.nfm");

    CollisionShape* pSponzaShape = ENGINE_GET_COLLISION_SHAPE("sponza_collision_shape.nfcs");
    //pSponzaShape->Load();

    BodyComponent* pFloorBody = new BodyComponent(pEnt);
    pFloorBody->EnablePhysics(pSponzaShape);
    pFloorBody->SetMass(0.0);

    pEnt = g_pScene->CreateEntity();
    pEnt->SetPosition(Vector(0.0f, 3.5f, 0.0f));
    LightComponent* pLight = new LightComponent(pEnt);
    OmniLightDesc omni;
    omni.m_ShadowFadeStart = 12.0f;
    omni.m_ShadowFadeEnd = 120.0f;
    omni.m_Radius = 90.0f;
    pLight->SetOmniLight(&omni);
    pLight->SetColor(Float3(50, 50, 50));
    pLight->SetShadowMap(512);
    /*
    // SUNLIGHT
    Entity* pDirLightEnt = g_pScene->CreateEntity();
    XOrientation orient;
    orient.x = Vector(0.0f, -0.0f, -0.0f, 0.0f);
    orient.z = Vector(0.1, -2.3f, 1.05, 0.0f);
    orient.y = Vector(0.0f, 1.0f, 0.0f, 0.0f);
    pDirLightEnt->SetOrientation(&orient);
    DirLightDesc dirLight;
    dirLight.m_Far = 100.0f;
    dirLight.m_Splits = 4;
    dirLight.m_LightDist = 1000.0;

    LightComponent* pDirLight = new LightComponent(pDirLightEnt);
    pDirLight->SetDirLight(&dirLight);
    pDirLight->SetColor(Float3(2.2, 1.3, 0.8));
    pDirLight->SetShadowMap(2048);
    */
#endif

//performance test (many objects and shadowmaps)
#ifdef SCENE_SEGMENTS_PERF_TEST
    for (int x = -4; x < 5; x++)
    {
        for (int z = -4; z < 5; z++)
        {
            Entity* pEntity = g_pScene->CreateEntity();
            pEntity->SetPosition(12.0f * Vector((float)x, 0, (float)z));
            MeshComponent* pMesh = new MeshComponent(pEntity);
            pMesh->SetMeshResource("chamber.nfm");
            BodyComponent* pBody = new BodyComponent(pEntity);
            pBody->EnablePhysics(pChamberShape);


            LightComponent* pLight;
            OmniLightDesc omni;
            pEntity = g_pScene->CreateEntity();
            pEntity->SetPosition(12.0f * Vector(x, 0, z) + Vector(0.0f, 3.5f, 0.0f));
            pLight = new LightComponent(pEntity);

            omni.shadowFadeStart = 80.0f;
            omni.shadowFadeEnd = 120.0f;
            omni.radius = 8.0f;
            pLight->SetOmniLight(&omni);
            pLight->SetColor(Float3(50, 50, 50));
            pLight->SetShadowMap(32);


            pEntity = g_pScene->CreateEntity();
            pEntity->SetPosition(12.0f * Vector(x, 0, z) + Vector(6.0f, 1.8f, 0.0f));
            pLight = new LightComponent(pEntity);
            omni.radius = 3.0f;
            pLight->SetOmniLight(&omni);
            pLight->SetColor(Float3(5.0f, 0.5f, 0.25f));

            pEntity = g_pScene->CreateEntity();
            pEntity->SetPosition(12.0f * Vector(x, 0, z) + Vector(0.0f, 1.8f, 6.0f));
            pLight = new LightComponent(pEntity);
            omni.radius = 3.0f;
            pLight->SetOmniLight(&omni);
            pLight->SetColor(Float3(5.0f, 0.5f, 0.25f));


            /*
            for (int i = -3; i<=3; i++)
            {
            for (int j = 0; j<4; j++)
            {
                for (int k = -3; k<=3; k++)
                {
                    Entity* pCube = g_pScene->CreateEntity();
                    pCube->SetPosition(12.0f * Vector(x,0,z) + 0.6f * Vector(i,j,k) + Vector(0.0f, 0.25f, 0.0f));

                    MeshComponent* pMesh = new MeshComponent(pCube);
                    pMesh->SetMeshResource("cube.nfm");

                    BodyComponent* pBody = new BodyComponent(pCube);
                    pBody->SetMass(0.0f);
                    pBody->EnablePhysics((CollisionShape*)Engine_GetResource(Mesh::COLLISION_SHAPE, "shape_box"));
                }
            }
            }*/
        }
    }
#endif

// infinite looped scene
#ifdef SCENE_SEGMENTS
    BufferOutputStream segmentDesc;

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
        pBody->EnablePhysics(pChamberShape);
        entity.Serialize(&segmentDesc, Vector());

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(0.0f, 3.5f, 0.0f));
        /*
            pLight = new LightComponent(&entity);
            omni.m_ShadowFadeEnd = 12.0f;
            omni.m_ShadowFadeStart = 8.0f;
            omni.m_Radius = 9.0f;
            pLight->SetOmniLight(&omni);
            pLight->SetColor(Float3(50, 50, 50));
            pLight->SetShadowMap(512);
                entity.Serialize(&segmentDesc, Vector());
                */
        entity.RemoveAllComponents();
        entity.SetPosition(Vector(6.0f, 1.8f, 0.0f));
        pLight = new LightComponent(&entity);
        omni.m_Radius = 3.0f;
        pLight->SetOmniLight(&omni);
        pLight->SetColor(Float3(5.0f, 0.5f, 0.25f));
        entity.Serialize(&segmentDesc, Vector());

        entity.RemoveAllComponents();
        entity.SetPosition(Vector(0.0f, 1.8f, 6.0f));
        pLight = new LightComponent(&entity);
        omni.m_Radius = 3.0f;
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
            pSegments[i][j] = g_pScene->CreateSegment(segName, Vector(5.99f, 1000.0f, 5.99f));
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
                g_pScene->CreateLink(pSegments[x][z], pSegments[(x + depth) % SEG_AXIS_NUM][z],
                                     Vector(depth * 12.0f, 0.0f, 0.0f));
                g_pScene->CreateLink(pSegments[x][z], pSegments[x][(z + depth) % SEG_AXIS_NUM], Vector(0.0, 0.0f,
                                     depth * 12.0f));
            }
        }
    }

    // Set focus
    g_pScene->SetFocusSegment(pSegments[0][0]);
#endif

    /*
    for (int i = -4; i<4; i++)
    {
        for (int j = -10; j<10; j++)
        {
            for (int k = -4; k<4; k++)
            {
                Entity* pCube = g_pScene->CreateEntity();
                pCube->SetPosition(0.75f * Vector(i,j,k));

                MeshComponent* pMesh = new MeshComponent(pCube);
                pMesh->SetMeshResource("cube.nfm");

                BodyComponent* pBody = new BodyComponent(pCube);
                pBody->SetMass(1.0f);
                pBody->EnablePhysics((CollisionShape*)Engine_GetResource(Mesh::COLLISION_SHAPE, "shape_box"));
            }
        }
    }

    //set ambient & background color
    envDesc.m_AmbientLight = Vector(0.001f, 0.001f, 0.001f, 0.0f);
    envDesc.m_BackgroundColor = Vector(0.0f, 0.0f, 0.0f, 0.0f);
    g_pScene->SetEnvironment(&envDesc);

    // SUNLIGHT
    Entity* pDirLightEnt = g_pScene->CreateEntity();
    XOrientation orient;
    orient.x = Vector(0.0f, -0.0f, -0.0f, 0.0f);
    orient.z = Vector(-0.5f, -1.1f, 1.2f, 0.0f);
    orient.y = Vector(0.0f, 1.0f, 0.0f, 0.0f);
    pDirLightEnt->SetOrientation(&orient);
    DirLightDesc dirLight;
    dirLight.m_Far = 100.0f;
    dirLight.m_Splits = 4;
    dirLight.m_LightDist = 1000.0f;

    LightComponent* pDirLight = new LightComponent(pDirLightEnt);
    pDirLight->SetDirLight(&dirLight);
    pDirLight->SetColor(Float3(2.2, 2, 1.8));
    pDirLight->SetShadowMap(1024);

    pFirstWindow->cameraEntity->SetPosition(Vector(0.0f, 1.6f, -20.0f, 0.0f));
    */

    // message loop

    DrawRequest drawRequests[2];

    Common::Timer timer;
    timer.Start();
    while (!pWindow->isClosed())
    {
        //measure delta time
        g_DeltaTime = (float)timer.Stop();
        timer.Start();

        UpdateRequest updateReq;
        updateReq.pScene = g_pScene;
        updateReq.deltaTime = g_DeltaTime;


        pWindow->processMessages();
        pWindow->UpdateCamera();

        drawRequests[0].deltaTime = g_DeltaTime;
        drawRequests[0].pView = pWindow->view;
        drawRequests[1].deltaTime = g_DeltaTime;
        drawRequests[1].pView = g_pSecondaryCameraView;
        EngineAdvance(drawRequests, 2, &updateReq, 1);

        ProcessSceneEvents();

        // print focus segment name
        wchar_t str[128];
        Segment* pFocus = g_pScene->GetFocusSegment();
        swprintf(str, L"NFEngine Demo (%S) - focus: %S", PLATFORM_STR,
                 (pFocus != 0) ? pFocus->GetName() : "NONE");
        pWindow->setTitle(str);
    }

    // for testing purposes
    Common::FileOutputStream test_stream("test.xml");
    g_pScene->Serialize(&test_stream, SerializationFormat::Xml, false);

    EngineDeleteScene(g_pScene);
    delete pWindow;
    EngineRelease();

//detect memory leaks
#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}