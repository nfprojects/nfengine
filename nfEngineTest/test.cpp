#include "stdafx.hpp"
#include "test.hpp"
#include "Editor.hpp"
#include "Scenes.hpp"

#include "../nfEngine/nfCommon/Logger.hpp"

using namespace NFE;
using namespace NFE::Render;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

class CustomWindow;

Random gRandom;
float gDeltaTime = 0.0f;

#ifdef WIN64
#define PLATFORM_STR "x64"
#else
#define PLATFORM_STR "x86"
#endif

MainCameraView::MainCameraView()
{
}

void MainCameraView::OnPostRender(IRenderContext* ctx, IGuiRenderer* guiRenderer)
{
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
        InitCameraOrientation();
    }

    ~CustomWindow()
    {
        delete view;
        // TODO: releasing engine before destruction of View without memory leak !!!
    }

    void InitCameraOrientation()
    {
        cameraControl = 0;
        cameraXZ = 0.0f;
        cameraY = 0.0f;
        cameraOrientation = QuaternionMultiply(QuaternionRotationY(cameraXZ),
                                               QuaternionRotationX(-cameraY));
    }

    // create camera entity, etc.
    void InitCamera()
    {
        InitCameraOrientation();
        cameraEntity = gScene->CreateEntity();
        cameraEntity->SetPosition(Vector(0.0f, 1.6f, -2.0f, 0.0f));

        UpdateCamera();
        Perspective perspective;
        perspective.FoV = NFE_MATH_PI * 60.0f / 180.0f;
        perspective.nearDist = 0.01f;
        perspective.farDist = 1000.0f;
        perspective.aspectRatio = GetAspectRatio();

        camera = new Camera(cameraEntity);
        camera->SetPerspective(&perspective);
        gScene->SetDefaultCamera(camera);

        BodyComponent* cameraBody = new BodyComponent(cameraEntity);

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
                            gDeltaTime / (CAMERA_ROTATION_SMOOTHING + gDeltaTime));
        cameraOrientation = QuaternionNormalize(cameraOrientation);

        Quaternion rotation = QuaternionMultiply(prevOrientation, QuaternionInverse(cameraOrientation));
        cameraEntity->SetAngularVelocity(-QuaternionToAxis(rotation) / gDeltaTime);

        Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(cameraOrientation));
        XOrientation orient;
        orient.x = rotMatrix.r[0];
        orient.y = rotMatrix.r[1];
        orient.z = rotMatrix.r[2];
        cameraEntity->SetOrientation(&orient);


        Vector destVelocity = Vector();
        if (cameraControl)
        {
            if (IsKeyPressed('W')) destVelocity += orient.z;
            if (IsKeyPressed('S')) destVelocity -= orient.z;
            if (IsKeyPressed('D')) destVelocity += orient.x;
            if (IsKeyPressed('A')) destVelocity -= orient.x;
            if (IsKeyPressed('R')) destVelocity += orient.y;
            if (IsKeyPressed('F')) destVelocity -= orient.y;

            if (IsKeyPressed(VK_SHIFT)) destVelocity *= 30.0f;
            else if (IsKeyPressed(VK_CONTROL)) destVelocity *= 0.2f;
            else destVelocity *= 3.0f;
        }

        Vector prevVelocity = cameraEntity->GetVelocity();

        //low pass filter - for smooth camera movement
        float factor = gDeltaTime / (CAMERA_TRANSLATION_SMOOTHING + gDeltaTime);
        cameraEntity->SetVelocity(VectorLerp(prevVelocity, destVelocity, factor));
    }

    void OnKeyPress(int key)
    {
        if (key == VK_F1)
        {
            BOOL fullscreen = GetFullscreenMode();
            SetFullscreenMode(!fullscreen);
        }

        XOrientation orient;

        //place spot light
        if (key == 'T')
        {
            Entity* lightEntity = gScene->CreateEntity();
            cameraEntity->GetOrientation(&orient);
            lightEntity->SetOrientation(&orient);
            lightEntity->SetPosition(cameraEntity->GetPosition());

            LightComponent* light = new LightComponent(lightEntity);
            SpotLightDesc lightDesc;
            lightDesc.nearDist = 0.1f;
            lightDesc.farDist = 500.0f;
            lightDesc.cutoff = NFE_MATH_PI / 4.0f;
            lightDesc.maxShadowDistance = 60.0;
            light->SetSpotLight(&lightDesc);
            light->SetColor(Float3(600, 200, 50));
            light->SetLightMap("flashlight.jpg");
            light->SetShadowMap(1024);

            gSelectedEntity = lightEntity;
        }

        //place omni light
        if (key == 'O')
        {
            OmniLightDesc lightDesc;
            lightDesc.radius = 10.0f;
            lightDesc.shadowFadeStart = 20.0;
            lightDesc.shadowFadeEnd = 30.0;

            Entity* lightEntity = gScene->CreateEntity();
            lightEntity->SetPosition(cameraEntity->GetPosition());

            LightComponent* light = new LightComponent(lightEntity);
            light->SetOmniLight(&lightDesc);
            light->SetColor(Float3(600, 600, 600));
            light->SetShadowMap(512);

            gSelectedEntity = lightEntity;
        }

        if (key >= '0' && key <= '9')
        {
            gSelectedEntity = nullptr;
            EngineDeleteScene(gScene);
            gScene = EngineCreateScene();
            InitCamera();
            InitScene(key - '0');
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
            Entity* cube = gScene->CreateEntity();
            gSelectedEntity = cube;
            cube->SetPosition(cameraEntity->GetPosition() + camOrient.z);
            cube->SetVelocity(0.1f * camOrient.z);
            //cube->SetMaxLifeTime(5.0f);

            MeshComponent* mesh = new MeshComponent(cube);
            mesh->SetMeshResource("cube.nfm");

            BodyComponent* body = new BodyComponent(cube);
            body->SetMass(10.0f);
            body->EnablePhysics((CollisionShape*)EngineGetResource(ResourceType::COLLISION_SHAPE,
                                 "shape_box"));


            OmniLightDesc lightDesc;
            lightDesc.radius = 4.0f;
            lightDesc.shadowFadeStart = 20.0;
            lightDesc.shadowFadeEnd = 30.0;

            Entity* lightEntity = gScene->CreateEntity();
            cube->Attach(lightEntity);
            //lightEntity->SetLocalPosition(Vector(0.0f, 1.0f, 0.0f));

            LightComponent* light = new LightComponent(lightEntity);
            light->SetOmniLight(&lightDesc);
            light->SetColor(Float3(1.0f, 1.0f, 10.0f));
            light->SetShadowMap(0);

        }
        else
        {
            Entity* barrel = gScene->CreateEntity();
            gSelectedEntity = barrel;
            barrel->SetPosition(cameraEntity->GetPosition() + camOrient.z);
            barrel->SetVelocity(30.0f * camOrient.z);

            MeshComponent* mesh = new MeshComponent(barrel);
            mesh->SetMeshResource("barrel.nfm");

            BodyComponent* body = new BodyComponent(barrel);
            body->SetMass(20.0f);
            body->EnablePhysics((CollisionShape*)EngineGetResource(ResourceType::COLLISION_SHAPE,
                                 "shape_barrel"));

        }
    }

    void OnMouseMove(int x, int y, int deltaX, int deltaY)
    {
        if (cameraControl)
        {
            float fDeltaX = static_cast<float>(deltaX) * 0.005f;
            float fDeltaY = static_cast<float>(deltaY) * 0.005f;

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
                perspective.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
                camera->SetPerspective(&perspective);
            }
        }
    }
};

void ProcessSceneEvents()
{
    SceneEvent eventID;
    void* data;
    while (gScene->GetEventSystem()->Pop(&eventID, &data))
    {
        switch (eventID)
        {
            case SceneEvent::EntityDestroy:
            {
                EventEntityDestroy* event = (EventEntityDestroy*)data;
                LOG_INFO("Entity destroyed: %p", event->entity);
                if (event->entity == gSelectedEntity)
                    gSelectedEntity = nullptr;
                break;
            }

            case SceneEvent::BodyCollide:
            {
                EventBodyCollide* event = (EventBodyCollide*)data;
                /*
                if (event->pBodyA->GetMass() > NFE_MATH_EPSILON)
                    gScene->EnqueueDeleteEntity(event->pBodyA->GetOwner());
                else
                    gScene->EnqueueDeleteEntity(event->pBodyB->GetOwner());
                */
                break;
            }
        }
    }
}

// temporary
bool OnLoadCustomShapeResource(ResourceBase* res, void* data)
{
    CollisionShape* shape = (CollisionShape*)res;

    if (strcmp(shape->GetName(), "shape_floor") == 0)
    {
        shape->AddBox(Vector(100.0f, 10.0f, 100.0f), MatrixIdentity());
    }
    else if (strcmp(shape->GetName(), "shape_box") == 0)
    {
        shape->AddBox(Vector(0.25f, 0.25f, 0.25f), MatrixIdentity());
    }
    else if (strcmp(shape->GetName(), "shape_frame") == 0)
    {
        // Z axis
        shape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(-0.475f, -0.475f, 0.0f)));
        shape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(-0.475f, 0.475f, 0.0f)));
        shape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(0.475f, -0.475f, 0.0f)));
        shape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(0.475f, 0.475f, 0.0f)));

        // Y axis
        shape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(-0.475f, 0.0f, -0.475f)));
        shape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(-0.475f, 0.0f, 0.475f)));
        shape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(0.475f, 0.0f, -0.475f)));
        shape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(0.475f, 0.0f, 0.475f)));

        // X axis
        shape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, -0.475f, -0.475f)));
        shape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, -0.475f, 0.475f)));
        shape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, 0.475f, -0.475f)));
        shape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, 0.475f, 0.475f)));
    }
    else if (strcmp(shape->GetName(), "shape_barrel") == 0)
    {
        shape->AddCylinder(1.31f, 0.421f);
    }

    return true;
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
    CustomWindow* window = new CustomWindow;
    window->SetSize(800, 600);
    window->SetTitle(L"NFEngine Demo - Initializing engine...");
    window->Open();

    //initialize engine
    if (EngineInit() != Result::OK)
        return 1;

    Demo_InitEditorBar();

    //create scene and camera
    gScene = EngineCreateScene();

    CollisionShape* floorShape = ENGINE_GET_COLLISION_SHAPE("shape_floor");
    floorShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    floorShape->Load();
    floorShape->AddRef();

    CollisionShape* frameShape = ENGINE_GET_COLLISION_SHAPE("shape_frame");
    frameShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    frameShape->Load();
    frameShape->AddRef();

    CollisionShape* boxShape = ENGINE_GET_COLLISION_SHAPE("shape_box");
    boxShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    boxShape->Load();
    boxShape->AddRef();

    CollisionShape* barrelShape = ENGINE_GET_COLLISION_SHAPE("shape_barrel");
    barrelShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    barrelShape->Load();
    barrelShape->AddRef();

    CollisionShape* chamberShape = ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs");
    chamberShape->Load();
    chamberShape->AddRef();


    window->InitCamera();
    window->SetTitle(L"NFEngine Demo");

    InitScene(0);

    // message loop

    DrawRequest drawRequest;

    Common::Timer timer;
    timer.Start();
    while (!window->IsClosed())
    {
        //measure delta time
        gDeltaTime = static_cast<float>(timer.Stop());
        timer.Start();

        UpdateRequest updateReq;
        updateReq.pScene = gScene;
        updateReq.deltaTime = gDeltaTime;

        window->ProcessMessages();
        window->UpdateCamera();

        drawRequest.deltaTime = gDeltaTime;
        drawRequest.pView = window->view;
        EngineAdvance(&drawRequest, 1, &updateReq, 1);

        ProcessSceneEvents();

        // print focus segment name
        wchar_t str[128];
        Segment* focus = gScene->GetFocusSegment();
        swprintf(str, L"NFEngine Demo (%S) - focus: %S - Press [0-%i] to switch scene",
                 PLATFORM_STR, (focus != 0) ? focus->GetName() : "NONE",
                 GetScenesNum() - 1);
        window->SetTitle(str);
    }

    EngineDeleteScene(gScene);
    delete window;
    EngineRelease();

//detect memory leaks
#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}