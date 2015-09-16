#include "PCH.hpp"
#include "Test.hpp"
#include "Editor.hpp"
#include "Scenes.hpp"

#include "../nfCommon/Window.hpp"
#include "../nfEngine/nfCommon/Logger.hpp"
#include "../nfCommon/FileSystem.hpp"

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

class CustomWindow;

Engine* gEngine = nullptr;
EntityManager* gEntityManager = nullptr;
Random gRandom;
float gDeltaTime = 0.0f;

#ifdef WIN64
#define PLATFORM_STR "x64"
#else
#define PLATFORM_STR "x86"
#endif

class MainCameraView : public NFE::Renderer::View
{
public:
    bool drawSecondaryView;

    MainCameraView()
        : drawSecondaryView(false)
    {}

    void OnPostRender(RenderContext* ctx)
    {
        if (!drawSecondaryView)
            return;

        float left = 50.0f;
        float bottom = 50.0f;
        float width = 256.0f;
        float height = 256.0f;
        float border = 2.0f;

        // border
        GuiRenderer::Get()->DrawQuad(ctx,
                                     Rectf(left - border, bottom - border,
                                           left + width + border, bottom + height + border),
                                     0xAA000000);

        // draw quad with secondary camera view

        Texture* texture = ENGINE_GET_TEXTURE("secondaryViewTexture");
        GuiRenderer::Get()->DrawTexturedQuad(ctx,
                                             Rectf(left, bottom, left + width, bottom + height),
                                             Rectf(0.0f, 0.0f, 1.0f, 1.0f),
                                             texture->GetRendererTexture(),
                                             0xFFFFFFFF);
    }
};

// overload own callback functions
class CustomWindow : public Common::Window
{
public:
    Quaternion cameraOrientation;
    EntityID cameraEntity;
    std::unique_ptr<MainCameraView> view;

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
        view = 0;
        InitCameraOrientation();
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
        cameraEntity = gEntityManager->CreateEntity();

        TransformComponent transform;
        transform.SetPosition(Vector(0.0f, 1.6f, -2.0f, 0.0f));
        gEntityManager->AddComponent(cameraEntity, transform);

        UpdateCamera();
        Perspective perspective;
        perspective.FoV = NFE_MATH_PI * 60.0f / 180.0f;
        perspective.nearDist = 0.01f;
        perspective.farDist = 1000.0f;
        perspective.aspectRatio = GetAspectRatio();
        CameraComponent camera;
        camera.SetPerspective(&perspective);
        gEntityManager->AddComponent(cameraEntity, camera);

        BodyComponent body;
        gEntityManager->AddComponent(cameraEntity, body);

        view.reset(new MainCameraView);
        view->SetCamera(gScene, cameraEntity);
        view->SetWindow(this);
    }

#define CAMERA_ROTATION_SMOOTHING 0.05f
#define CAMERA_TRANSLATION_SMOOTHING 0.2f

    // update camera position and orientation
    void UpdateCamera()
    {
        auto cameraBody = gEntityManager->GetComponent<BodyComponent>(cameraEntity);
        auto cameraTransform = gEntityManager->GetComponent<TransformComponent>(cameraEntity);
        if (cameraBody == nullptr || cameraTransform == nullptr)
            return;

        Quaternion destOrientation = QuaternionMultiply(QuaternionRotationY(cameraXZ),
                                     QuaternionRotationX(-cameraY));
        destOrientation = QuaternionNormalize(destOrientation);

        //LPF
        Quaternion prevOrientation = cameraOrientation;
        cameraOrientation = QuaternionInterpolate(cameraOrientation, destOrientation,
                            gDeltaTime / (CAMERA_ROTATION_SMOOTHING + gDeltaTime));
        cameraOrientation = QuaternionNormalize(cameraOrientation);

        Quaternion rotation = QuaternionMultiply(prevOrientation,
                                                 QuaternionInverse(cameraOrientation));
        cameraBody->SetAngularVelocity(-QuaternionToAxis(rotation) / gDeltaTime);

        Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(cameraOrientation));
        Orientation orient;
        orient.x = rotMatrix.r[0];
        orient.y = rotMatrix.r[1];
        orient.z = rotMatrix.r[2];
        cameraTransform->SetOrientation(orient);


        Vector destVelocity = Vector();
        if (cameraControl)
        {
            if (IsKeyPressed('W')) destVelocity += orient.z;
            if (IsKeyPressed('S')) destVelocity -= orient.z;
            if (IsKeyPressed('D')) destVelocity += orient.x;
            if (IsKeyPressed('A')) destVelocity -= orient.x;
            if (IsKeyPressed('R')) destVelocity += orient.y;
            if (IsKeyPressed('F')) destVelocity -= orient.y;

            if (IsKeyPressed(Common::KeyCode::Shift)) destVelocity *= 30.0f;
            else if (IsKeyPressed(Common::KeyCode::Control)) destVelocity *= 0.2f;
            else destVelocity *= 3.0f;
        }

        Vector prevVelocity = cameraBody->GetVelocity();

        // low pass filter - for smooth camera movement
        float factor = gDeltaTime / (CAMERA_TRANSLATION_SMOOTHING + gDeltaTime);
        cameraBody->SetVelocity(VectorLerp(prevVelocity, destVelocity, factor));
    }

    void OnKeyPress(int key)
    {
        if (key == Common::KeyCode::F1)
        {
            BOOL fullscreen = GetFullscreenMode();
            SetFullscreenMode(!fullscreen);
        }

        auto cameraTransform = gEntityManager->GetComponent<TransformComponent>(cameraEntity);
        Orientation orient;

        //place spot light
        if (key == 'T' && cameraTransform != nullptr)
        {
            EntityID lightEntity = gEntityManager->CreateEntity();
            gSelectedEntity = lightEntity;

            TransformComponent transform;
            transform.SetOrientation(cameraTransform->GetOrientation());
            transform.SetPosition(cameraTransform->GetPosition());
            gEntityManager->AddComponent(lightEntity, transform);

            LightComponent light;
            SpotLightDesc lightDesc;
            lightDesc.nearDist = 0.1f;
            lightDesc.farDist = 500.0f;
            lightDesc.cutoff = NFE_MATH_PI / 4.0f;
            lightDesc.maxShadowDistance = 60.0f;
            light.SetSpotLight(&lightDesc);
            light.SetColor(Float3(600.0f, 200.0f, 50.0f));
            light.SetLightMap("flashlight.jpg");
            light.SetShadowMap(1024);
            gEntityManager->AddComponent(lightEntity, light);
        }

        //place omni light
        if (key == 'O' && cameraTransform != nullptr)
        {
            OmniLightDesc lightDesc;
            lightDesc.radius = 10.0f;
            lightDesc.shadowFadeStart = 20.0f;
            lightDesc.maxShadowDistance = 30.0f;

            EntityID lightEntity = gEntityManager->CreateEntity();
            gSelectedEntity = lightEntity;

            TransformComponent transform;
            transform.SetPosition(cameraTransform->GetPosition());
            gEntityManager->AddComponent(lightEntity, transform);

            LightComponent light;
            light.SetOmniLight(&lightDesc);
            light.SetColor(Float3(600, 600, 600));
            light.SetShadowMap(512);
            gEntityManager->AddComponent(lightEntity, light);
        }

        if (key == 'V')
        {
            view->drawSecondaryView ^= true;
        }

        if (key >= '0' && key <= '9')
        {
            gSelectedEntity = -1;
            gEngine->DeleteScene(gScene);

            gScene = gEngine->CreateScene();
            gEntityManager = gScene->GetEntityManager();

            InitCamera();
            InitScene(key - '0');
        }
    }

    void OnMouseDown(UINT button, int x, int y)
    {
        if (button == 0)
            cameraControl = true;

        auto cameraTransform = gEntityManager->GetComponent<TransformComponent>(cameraEntity);
        if (cameraTransform == nullptr)
            return;
        Orientation camOrient = cameraTransform->GetOrientation();

        //shoot a cube
        if (button == 1)
        {
            EntityID cube = gEntityManager->CreateEntity();
            gSelectedEntity = cube;

            TransformComponent transform;
            transform.SetPosition(cameraTransform->GetPosition() + camOrient.z);
            gEntityManager->AddComponent(cube, transform);

            MeshComponent mesh;
            mesh.SetMeshResource("cube.nfm");
            gEntityManager->AddComponent(cube, mesh);

            BodyComponent body;
            body.SetMass(10.0f);
            body.SetVelocity(0.1f * camOrient.z);
            body.EnablePhysics(ENGINE_GET_COLLISION_SHAPE("shape_box"));
            gEntityManager->AddComponent(cube, body);

            {
                EntityID child = gEntityManager->CreateEntity();
                gSelectedEntity = cube;

                TransformComponent transform;
                transform.SetLocalPosition(Vector(0.0f, 1.0f, 0.0f));
                gEntityManager->AddComponent(child, transform);

                OmniLightDesc lightDesc;
                lightDesc.radius = 4.0f;
                lightDesc.shadowFadeStart = 20.0;
                lightDesc.maxShadowDistance = 30.0;
                LightComponent light;
                light.SetOmniLight(&lightDesc);
                light.SetColor(Float3(1.0f, 1.0f, 10.0f));
                light.SetShadowMap(0);
                gEntityManager->AddComponent(child, light);

                gScene->GetTransformSystem()->SetParent(child, cube);
            }
        }

        if (button == 2)
        {
            EntityID barrel = gEntityManager->CreateEntity();
            gSelectedEntity = barrel;

            TransformComponent transform;
            transform.SetPosition(cameraTransform->GetPosition() + camOrient.z);
            gEntityManager->AddComponent(barrel, transform);

            MeshComponent mesh;
            mesh.SetMeshResource("barrel.nfm");
            gEntityManager->AddComponent(barrel, mesh);

            BodyComponent body;
            body.SetMass(20.0f);
            body.SetVelocity(30.0f * camOrient.z);
            body.EnablePhysics(ENGINE_GET_COLLISION_SHAPE("shape_barrel"));
            gEntityManager->AddComponent(barrel, body);
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
        if (gEntityManager == nullptr)
            return;

        auto camera = gEntityManager->GetComponent<CameraComponent>(cameraEntity);
        if (camera != nullptr)
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

// temporary
bool OnLoadCustomShapeResource(ResourceBase* res, void* data)
{
    CollisionShape* shape = (CollisionShape*)res;

    if (strcmp(shape->GetName(), "shape_floor") == 0)
    {
        shape->AddBox(Vector(100.0f, 10.0f, 100.0f), Matrix());
    }
    else if (strcmp(shape->GetName(), "shape_box") == 0)
    {
        shape->AddBox(Vector(0.25f, 0.25f, 0.25f), Matrix());
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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    std::string execPath = Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    //create window
    CustomWindow* window = new CustomWindow;
    window->SetSize(800, 600);
    window->SetTitle("NFEngine Demo - Initializing engine...");
    window->Open();

    //initialize engine
    gEngine = Engine::GetInstance();
    if (gEngine == nullptr)
        return 1;

    Demo_InitEditorBar();

    //create scene and camera
    gScene = gEngine->CreateScene();
    if (gScene == nullptr)
        return 1;

    gEntityManager = gScene->GetEntityManager();
    if (gEntityManager == nullptr)
        return 1;

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
    window->SetTitle("NFEngine Demo");

    InitScene(0);

    // message loop

    Common::Timer timer;
    timer.Start();
    while (!window->IsClosed())
    {
        //measure delta time
        gDeltaTime = static_cast<float>(timer.Stop());
        timer.Start();

        window->ProcessMessages();
        window->UpdateCamera();

        UpdateRequest updateReq;
        updateReq.scene = gScene;
        updateReq.deltaTime = gDeltaTime;

        DrawRequest drawRequest;
        drawRequest.view = window->view.get();
        gEngine->Advance(&drawRequest, 1, &updateReq, 1);

        char str[128];
        sprintf(str, "NFEngine Demo (%s)  -  Press [0-%i] to switch scene",
                 PLATFORM_STR, GetScenesNum() - 1);
        window->SetTitle(str);
    }

    gEngine->DeleteScene(gScene);
    delete window;
    Engine::Release();

//detect memory leaks
#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
