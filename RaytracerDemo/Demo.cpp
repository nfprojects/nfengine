#include "PCH.h"
#include "Demo.h"
#include "SceneLoader.h"

#include "../nfEngine/nfCommon/System/Timer.hpp"
#include "../nfEngine/nfCommon/Logger/Logger.hpp"
#include "../nfEngine/Raytracer/Utils/Profiler.h"
#include "../nfEngine/Raytracer/Rendering/Renderer.h"
#include "../nfEngine/Raytracer/Rendering/Film.h"
#include "../nfEngine/Raytracer/Traversal/TraversalContext.h"
#include "../nfEngine/Raytracer/Textures/Texture.h"
#include "../nfEngine/Raytracer/Scene/Object/SceneObject.h"

#include "../nfEngineDeps/imgui/imgui.h"
#include "../nfEngineDeps/imgui/imgui_sw.hpp"

namespace NFE {

using namespace RT;
using namespace Math;
using namespace Common;

namespace helpers
{
extern bool LoadCustomScene(Scene& scene, Camera& camera);
}

DemoWindow::DemoWindow()
    : mLastKeyDown(KeyCode::Invalid)
    , mCameraSpeed(1.0f)
    , mFrameNumber(0)
    , mDeltaTime(0.0)
    , mRefreshTime(0.0)
    , mAverageRenderDeltaTime(0.0)
    , mMinimumRenderDeltaTime(0.0)
    , mAccumulatedRenderTime(0.0)
    , mRenderDeltaTime(0.0)
    , mTotalRenderTime(0.0)
    , mSelectedMaterial(nullptr)
    , mSelectedObject(nullptr)
{
    ResetFrame();
    ResetCounters();
}

DemoWindow::~DemoWindow()
{
    imgui_sw::unbind_imgui_painting();
}

bool DemoWindow::Initialize()
{
    NFE_LOG_INFO("Using data path: %hs", gOptions.dataPath.Str());

    if (!Init())
    {
        NFE_LOG_ERROR("Failed to init window");
        return false;
    }

    SetSize(gOptions.windowWidth, gOptions.windowHeight);
    SetTitle("Raytracer Demo [Initializing...]");

    if (!Open())
    {
        NFE_LOG_ERROR("Failed to open window");
        return false;
    }

    InitializeUI();

    mRenderingParams.traversalMode = gOptions.enablePacketTracing ? TraversalMode::Packet : TraversalMode::Single;

    mViewport = MakeUniquePtr<Viewport>();

    mViewport->Resize(gOptions.windowWidth, gOptions.windowHeight);

    NFE_LOG_INFO("%p", &mViewport->GetFrontBuffer());

    NFE_ASSERT(mViewport->GetFrontBuffer().GetWidth() == gOptions.windowWidth);
    NFE_ASSERT(mViewport->GetFrontBuffer().GetHeight() == gOptions.windowHeight);
    NFE_ASSERT(mViewport->GetFrontBuffer().GetFormat() == Bitmap::Format::B8G8R8A8_UNorm);

    Bitmap::InitData initData;
    initData.linearSpace = false;
    initData.width = gOptions.windowWidth;
    initData.height = gOptions.windowHeight;
    initData.format = Bitmap::Format::B8G8R8A8_UNorm;
    initData.useDefaultAllocator = true; // for some reason displaying a bitmap that uses large page fails
    mImage.Init(initData);

    mCamera.mDOF.aperture = 0.0f;

    SwitchScene(gOptions.sceneName);

    return true;
}

void DemoWindow::InitializeUI()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImFontConfig fontConfig;
    fontConfig.OversampleH = 4;
    fontConfig.OversampleV = 4;
    fontConfig.PixelSnapH = true;
    ImGui::GetIO().Fonts->AddFontFromFileTTF("../Data/Fonts/DroidSans-Regular.otf", 13, &fontConfig);

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;   // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;    // We can honor io.WantSetMousePos requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = (int)KeyCode::Tab;
    io.KeyMap[ImGuiKey_LeftArrow] = (int)KeyCode::Left;
    io.KeyMap[ImGuiKey_RightArrow] = (int)KeyCode::Right;
    io.KeyMap[ImGuiKey_UpArrow] = (int)KeyCode::Up;
    io.KeyMap[ImGuiKey_DownArrow] = (int)KeyCode::Down;
    io.KeyMap[ImGuiKey_PageUp] = (int)KeyCode::PageUp;
    io.KeyMap[ImGuiKey_PageDown] = (int)KeyCode::PageDown;
    io.KeyMap[ImGuiKey_Home] = (int)KeyCode::Home;
    io.KeyMap[ImGuiKey_End] = (int)KeyCode::End;
    io.KeyMap[ImGuiKey_Insert] = (int)KeyCode::Insert;
    io.KeyMap[ImGuiKey_Delete] = (int)KeyCode::Delete;
    io.KeyMap[ImGuiKey_Backspace] = (int)KeyCode::Backspace;
    io.KeyMap[ImGuiKey_Space] = (int)KeyCode::Space;
    io.KeyMap[ImGuiKey_Enter] = (int)KeyCode::Enter;
    io.KeyMap[ImGuiKey_Escape] = (int)KeyCode::Escape;
    io.KeyMap[ImGuiKey_A] = (int)KeyCode::A;
    io.KeyMap[ImGuiKey_C] = (int)KeyCode::C;
    io.KeyMap[ImGuiKey_V] = (int)KeyCode::V;
    io.KeyMap[ImGuiKey_X] = (int)KeyCode::X;
    io.KeyMap[ImGuiKey_Y] = (int)KeyCode::Y;
    io.KeyMap[ImGuiKey_Z] = (int)KeyCode::Z;

    ImGui::GetStyle().FramePadding = ImVec2(4, 2);
    ImGui::GetStyle().ItemSpacing = ImVec2(4, 2);
    ImGui::GetStyle().ItemInnerSpacing = ImVec2(2, 2);
    ImGui::GetStyle().ScrollbarSize = 13.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;
    ImGui::GetStyle().WindowRounding = 5.0f;
    ImGui::GetStyle().WindowPadding = ImVec2(6, 6);

    imgui_sw::bind_imgui_painting();
}

void DemoWindow::CheckSceneFileModificationTime()
{
    if (!mSceneFileName.Empty())
    {
        // read modification time
        struct stat fileStat;
        if (stat(mSceneFileName.Str(), &fileStat) == 0)
        {
            if (mSceneFileModificationTime != fileStat.st_mtime)
            {
                NFE_LOG_INFO("Scene file '%s' modified, reloading", mSceneFileName.Str());
                SwitchScene(mSceneFileName);
            }
        }
    }
}

void DemoWindow::SwitchScene(const String& sceneName)
{
    mScene = MakeUniquePtr<Scene>();

    if (!sceneName.Empty())
    {
        if (helpers::LoadScene(sceneName, *mScene, mCamera))
        {
            mSceneFileName = sceneName;

            // read modification time
            struct stat fileStat;
            if (stat(sceneName.Str(), &fileStat) == 0)
            {
                mSceneFileModificationTime = fileStat.st_mtime;
            }
        }
        else
        {
            mSceneFileName.Clear();
        }
    }
    else
    {
        helpers::LoadCustomScene(*mScene, mCamera);
        mSceneFileName.Clear();
    }

    mCameraSetup.position = mCamera.mTransform.GetTranslation();
    mCameraSetup.orientation = mCamera.mTransform.GetRotation().ToEulerAngles();
    mCameraSetup.fov = RadToDeg(mCamera.mFieldOfView);

    mScene->BuildBVH();
    ResetCounters();
    ResetFrame();

    mSelectedMaterial = nullptr;
    mSelectedObject = nullptr;

    mRenderer = CreateRenderer(gOptions.rendererName, *mScene);
    mViewport->SetRenderer(mRenderer.Get());
}

void DemoWindow::ResetFrame()
{
    if (mViewport)
    {
        mViewport->Reset();
    }

    ResetCounters();
}

void DemoWindow::ResetCounters()
{
    mFrameNumber = 0;
    mFrameCounterForAverage = 0;
    mAccumulatedRenderTime = 0.0;
    mAverageRenderDeltaTime = 0.0;
    mMinimumRenderDeltaTime = std::numeric_limits<double>::max();
    mTotalRenderTime = 0.0;
}

void DemoWindow::OnResize(uint32 width, uint32 height)
{
    if (mViewport)
    {
        mViewport->Resize(width, height);

        Bitmap::InitData initData;
        initData.linearSpace = true;
        initData.width = width;
        initData.height = height;
        initData.format = Bitmap::Format::B8G8R8A8_UNorm;
        initData.useDefaultAllocator = true; // for some reason displaying a bitmap that uses large page fails
        mImage.Init(initData);
    }

    UpdateCamera();
    ResetCounters();
}

void DemoWindow::OnMouseDown(MouseButton button, int x, int y)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[(int)button] = true;

    if (io.WantCaptureMouse)
        return;

    uint32 width, height;
    GetSize(width, height);

    RenderingParams params = mRenderingParams;
    params.antiAliasingSpread = 0.0f;

    auto renderingContext = MakeUniquePtr<RenderingContext>();
    renderingContext->params = &params;
    renderingContext->sampler.fallbackGenerator = &renderingContext->randomGenerator;
    renderingContext->rendererContext = mRenderer->CreateContext();

    if (mFocalDistancePicking && button == MouseButton::Left)
    {
        const Vector4 coords((float)x / (float)width, 1.0f - (float)y / (float)height, 0.0f, 0.0f);
        const Ray ray = mCamera.GenerateRay(coords, *renderingContext);

        HitPoint hitPoint;
        mScene->Traverse({ ray, hitPoint, *renderingContext });

        if (hitPoint.distance == FLT_MAX)
        {
            mCamera.mDOF.focalPlaneDistance = 10000000.0f;
        }
        else
        {
            mCamera.mDOF.focalPlaneDistance = Vector4::Dot3(mCamera.mTransform.GetRotation().GetAxisZ(), ray.dir) * hitPoint.distance;
        }
        ResetFrame();

        mFocalDistancePicking = false;
    }
    else if (mPixelDebuggingPicking)
    {
        mPixelDebuggingPicking = false;
        mViewport->SetPixelBreakpoint(x, y);
    }
    else if (button == MouseButton::Left)
    {
        const Vector4 coords((float)x / (float)width, 1.0f - (float)y / (float)height);
        const Ray ray = mCamera.GenerateRay(coords, *renderingContext);

#ifndef RT_CONFIGURATION_FINAL
        mPathDebugData.Clear();
        renderingContext->pathDebugData = &mPathDebugData;

        // render pixel in order to obtain path debug data
        {
            Film fakeFilm;
            IRenderer::RenderParam renderParam = { *mScene, mCamera, 0, fakeFilm };
            mRenderer->RenderPixel(ray, renderParam, *renderingContext);
        }
#endif

        HitPoint hitPoint;
        mScene->Traverse({ ray, hitPoint, *renderingContext });

        if (hitPoint.objectId != UINT32_MAX)
        {
            if (hitPoint.subObjectId == NFE_LIGHT_OBJECT)
            {
                mSelectedMaterial = nullptr;
            }
            else // regular scene object
            {
                IntersectionData intersectionData;
                mScene->EvaluateIntersection(ray, hitPoint, renderingContext->time, intersectionData);
                mSelectedMaterial = const_cast<Material*>(intersectionData.material);
            }
            mSelectedObject = const_cast<ITraceableSceneObject*>(mScene->GetHitObject(hitPoint.objectId));
        }
    }
}

void DemoWindow::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = (float)x;
    io.MousePos.y = (float)y;

    if (IsMouseButtonDown(MouseButton::Right))
    {
        const float sensitivity = 0.0001f * mCameraSetup.fov;
        mCameraSetup.orientation.y += sensitivity * (float)deltaX;
        mCameraSetup.orientation.x += sensitivity * (float)deltaY;

        // clamp yaw
        if (mCameraSetup.orientation.y > NFE_MATH_PI)   mCameraSetup.orientation.y -= 2.0f * NFE_MATH_PI;
        if (mCameraSetup.orientation.y < -NFE_MATH_PI)  mCameraSetup.orientation.y += 2.0f * NFE_MATH_PI;

        // clamp pitch
        if (mCameraSetup.orientation.x > NFE_MATH_PI * 0.49f)     mCameraSetup.orientation.x = NFE_MATH_PI * 0.49f;
        if (mCameraSetup.orientation.x < -NFE_MATH_PI * 0.49f)    mCameraSetup.orientation.x = -NFE_MATH_PI * 0.49f;
    }
}

void DemoWindow::OnMouseUp(MouseButton button)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[(int)button] = false;
}

void DemoWindow::OnScroll(int delta)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = (float)delta;

    const float speedMultiplier = 1.25f;

    if (delta > 0)
    {
        mCameraSpeed *= speedMultiplier;
    }
    else if (delta < 0)
    {
        mCameraSpeed /= speedMultiplier;
    }
}

void DemoWindow::OnKeyPress(KeyCode key)
{
    if (key == KeyCode::U)
    {
        mEnableUI = !mEnableUI;
    }
    else if (key == KeyCode::P && IsMouseButtonDown(MouseButton::Right))
    {
        mViewport->GetFrontBuffer().SaveBMP("screenshot.bmp", true);
    }

    mLastKeyDown = key;
}

void DemoWindow::OnCharTyped(const char* charUTF8)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharactersUTF8(charUTF8);
}

void DemoWindow::OnFileDrop(const String& filePath)
{
    SwitchScene(filePath);
}

bool DemoWindow::Loop()
{
    Timer localTimer;
    Timer displayTimer;
    displayTimer.Start();

    char buffer[64];

    while (!IsClosed())
    {
        FrameMark;

        CheckSceneFileModificationTime();

        const RenderingProgress& progress = mViewport->GetProgress();
        sprintf(buffer, "Raytracer Demo [%.1f%% converged, pass %u, dt: %.2f]", 100.0f * progress.converged, progress.passesFinished, 1000.0f * mDeltaTime);
        SetTitle(buffer);

        mDeltaTime = displayTimer.Restart();

        UpdateCamera();

        bool resetFrame = false;

        if (IsPreview() && mViewport)
        {
            mPreviewRenderingParams = mRenderingParams;
            mPreviewRenderingParams.antiAliasingSpread = 0.0f;
            resetFrame |= true;
        }

        if (mEnableUI)
        {
            resetFrame |= RenderUI();
        }

        mViewport->SetRenderingParams(IsPreview() ? mPreviewRenderingParams : mRenderingParams);

        if (resetFrame)
        {
            ResetFrame();
        }

        //// render
        localTimer.Start();
        mViewport->Render(*mScene, mCamera);
        mRenderDeltaTime = localTimer.Stop();

        {
            NFE_SCOPED_TIMER(CopyFrontBuffer);
            Bitmap::Copy(mImage, mViewport->GetFrontBuffer());
        }

        if (mVisualizeAdaptiveRenderingBlocks)
        {
            mViewport->VisualizeActiveBlocks(mImage);
        }

        // render UI into the front buffer
        if (mEnableUI)
        {
            NFE_SCOPED_TIMER(PaintImGui);
            imgui_sw::paint_imgui((uint32_t*)mImage.GetData(), mImage.GetWidth(), mImage.GetHeight());
        }

        // display pixels in the window
        {
            NFE_SCOPED_TIMER(DrawPixels);
            DrawPixels(mImage.GetData(), mImage.GetWidth(), mImage.GetHeight(), mImage.GetStride());
        }

        mLastKeyDown = KeyCode::Invalid;

        mTotalRenderTime += mRenderDeltaTime;
        mRefreshTime += mRenderDeltaTime;
        mAccumulatedRenderTime += mRenderDeltaTime;
        mFrameCounterForAverage++;
        mFrameNumber++;
        mAverageRenderDeltaTime = mTotalRenderTime / (double)mFrameCounterForAverage;
        mMinimumRenderDeltaTime = mFrameNumber > 3 ? Math::Min(mMinimumRenderDeltaTime, mRenderDeltaTime) : mRenderDeltaTime;

        // handle window input
        ProcessMessages();
    }

    return true;
}

bool DemoWindow::IsPreview() const
{
    return IsMouseButtonDown(MouseButton::Right);
}

void DemoWindow::UpdateCamera()
{
    uint32 width, height;
    GetSize(width, height);

    const Camera oldCameraSetup = mCamera;

    // calculate camera direction from Euler angles
    const Quaternion cameraOrientation = Quaternion::FromEulerAngles(mCameraSetup.orientation);
    const Vector4 frontDir = cameraOrientation.GetAxisZ();
    const Vector4 rightDir = cameraOrientation.GetAxisX();
    const Vector4 upDir = cameraOrientation.GetAxisY();

    Vector4 movement = Math::Vector4::Zero();
    if (IsKeyPressed(KeyCode::W))
        movement += frontDir;
    if (IsKeyPressed(KeyCode::S))
        movement -= frontDir;
    if (IsKeyPressed(KeyCode::D))
        movement += rightDir;
    if (IsKeyPressed(KeyCode::A))
        movement -= rightDir;
    if (IsKeyPressed(KeyCode::R))
        movement += upDir;
    if (IsKeyPressed(KeyCode::F))
        movement -= upDir;

    // TODO
    //mCamera.mLinearVelocity = mCameraSetup.linearVelocity;

    if (movement.Length3() > NFE_MATH_EPSILON)
    {
        ResetFrame();

        movement.Normalize3();
        movement *= mCameraSpeed;

        if (IsKeyPressed(KeyCode::ShiftLeft))
            movement *= 5.0f;
        else if (IsKeyPressed(KeyCode::ControlLeft))
            movement /= 5.0f;

        const Vector4 delta = movement * (float)mDeltaTime;
        mCameraSetup.position += delta;

        // TODO
        //mCamera.mLinearVelocity -= delta;
    }

    mCamera.SetTransform(Transform(mCameraSetup.position, cameraOrientation));

    const float aspectRatio = (float)width / (float)height;
    const float FoV = DegToRad(mCameraSetup.fov);
    mCamera.SetPerspective(aspectRatio, FoV);

    // rotation motion blur
    if (IsPreview())
    {
        const Quaternion q1 = cameraOrientation.Conjugate();
        const Quaternion q2 = oldCameraSetup.mTransform.GetRotation();

        mCamera.SetAngularVelocity(q1 * q2);
    }
    else
    {
        mCamera.SetAngularVelocity(Quaternion::FromEulerAngles(mCameraSetup.angularVelocity));
    }
}

} // namespace NFE
