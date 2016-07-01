#pragma once

#include "../nfEngine/nfCommon/System/Window.hpp"
#include "../nfEngine/Raytracer/Scene/Scene.h"
#include "../nfEngine/Raytracer/Scene/Camera.h"
#include "../nfEngine/Raytracer/Rendering/Viewport.h"
#include "../nfEngine/Raytracer/Utils/Bitmap.h"
#include "../nfEngine/Raytracer/Rendering/Context.h"
#include "../nfEngine/Raytracer/Rendering/PathDebugging.h"

namespace NFE {

struct Options
{
    uint32 windowWidth = 1280;
    uint32 windowHeight = 720;
    Common::String dataPath;

    bool enablePacketTracing = false;
    Common::String rendererName = "Path Tracer";

    Common::String sceneName;
};

struct CameraSetup
{
    Math::Vector4 position = Math::Vector4::Zero();
    Math::Vector4 linearVelocity = Math::Vector4::Zero();
    Math::Float3 orientation; // yaw, pitch, roll
    Math::Float3 angularVelocity;
    float fov = 60.0f;
};


class DemoWindow : public Common::Window
{
public:
    DemoWindow();
    ~DemoWindow();

    bool Initialize();

    /**
     * Main loop.
     */
    bool Loop();

    void ResetFrame();

private:
    Common::UniquePtr<RT::Viewport> mViewport;
    RT::Bitmap mImage;

    Common::KeyCode mLastKeyDown;

    RT::Camera mCamera;
    RT::RenderingParams mRenderingParams;
    RT::RenderingParams mPreviewRenderingParams;
    RT::PostprocessParams mPostprocessParams;
    CameraSetup mCameraSetup;
    float mCameraSpeed;

    Common::UniquePtr<RT::Scene> mScene;

    uint32 mFrameNumber;
    uint32 mFrameCounterForAverage;

    double mDeltaTime;
    double mRefreshTime;

    double mAverageRenderDeltaTime;
    double mMinimumRenderDeltaTime;
    double mAccumulatedRenderTime;
    double mRenderDeltaTime;
    double mTotalRenderTime;

    Common::String mSceneFileName;
    time_t mSceneFileModificationTime;

    Common::String mRendererName;
    RT::RendererPtr mRenderer;

    bool mEnableUI = true;
    bool mVisualizeAdaptiveRenderingBlocks = false;

    // debugging
    RT::PathDebugData mPathDebugData;
    RT::Material* mSelectedMaterial;
    RT::ITraceableSceneObject* mSelectedObject;
    RT::ILight* mSelectedLight;
    bool mFocalDistancePicking = false;
    bool mPixelDebuggingPicking = false;

    void InitializeUI();

    void CheckSceneFileModificationTime();
    void SwitchScene(const Common::String& sceneName);

    bool RenderUI();
    void RenderUI_Stats();
    void RenderUI_Profiler();

    void RenderUI_Debugging();
    void RenderUI_Debugging_Path();
    void RenderUI_Debugging_Color();

    bool RenderUI_Settings();
    bool RenderUI_Settings_Rendering();
    bool RenderUI_Settings_Sampling();
    bool RenderUI_Settings_AdaptiveRendering();
    bool RenderUI_Settings_Camera();
    bool RenderUI_Settings_PostProcess();
    bool RenderUI_Settings_Object();
    bool RenderUI_Settings_Light();
    bool RenderUI_Settings_Material();


    virtual void OnMouseDown(Common::MouseButton button, int x, int y) override;
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY) override;
    virtual void OnMouseUp(Common::MouseButton button) override;
    virtual void OnScroll(int delta) override;
    virtual void OnResize(uint32 width, uint32 height) override;
    virtual void OnKeyPress(Common::KeyCode key) override;
    virtual void OnCharTyped(const char* charUTF8) override;
    virtual void OnFileDrop(const Common::String& filePath) override;

    bool IsPreview() const;
    void ResetCounters();
    void UpdateCamera();
};

extern Options gOptions;

} // namespace NFE