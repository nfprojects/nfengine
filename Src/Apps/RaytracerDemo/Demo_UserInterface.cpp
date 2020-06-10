#include "PCH.h"
#include "Demo.h"
#include "ObjectEditor.h"

#include "Engine/Raytracer/Material/Material.h"
#include "Engine/Raytracer/Scene/Object/SceneObject.h"
#include "Engine/Raytracer/Scene/Light/BackgroundLight.h"
#include "Engine/Raytracer/Rendering/Renderer.h"
#include "Engine/Raytracer/Utils/Profiler.h"

#include "Engine/Common/Reflection/Types/ReflectionUniquePtrType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionClassType.hpp"

#include "../nfEngineDeps/imgui/imgui.h"


namespace NFE {

using namespace Math;
using namespace RT;
using namespace Common;

static bool EditEulerAngles(const char* name, Vec3f& angles)
{
    Vec3f orientation = angles * (180.0f / NFE_MATH_PI);
    if (ImGui::InputFloat3(name, &orientation.x, 3, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        angles = orientation * (NFE_MATH_PI / 180.0f);
        return true;
    }
    return false;
}

void DemoWindow::RenderUI_Stats()
{
    const RenderingProgress& progress = mViewport->GetProgress();

    ImGui::Columns(2);

    ImGui::Text("Average render time"); ImGui::NextColumn();
    ImGui::Text("%.2f ms", 1000.0 * mAverageRenderDeltaTime); ImGui::NextColumn();

    ImGui::Text("Minimum render time"); ImGui::NextColumn();
    ImGui::Text("%.2f ms", 1000.0 * mMinimumRenderDeltaTime); ImGui::NextColumn();

    ImGui::Text("Total render time"); ImGui::NextColumn();
    ImGui::Text("%.3f s", mTotalRenderTime); ImGui::NextColumn();

    ImGui::Separator();

    ImGui::Text("Passes finished"); ImGui::NextColumn();
    ImGui::Text("%u", progress.passesFinished); ImGui::NextColumn();

    ImGui::Text("Error"); ImGui::NextColumn();
    ImGui::Text("%.3f dB", 10.0f * log10f(progress.averageError)); ImGui::NextColumn();

    ImGui::Text("Progress"); ImGui::NextColumn();
    ImGui::Text("%.2f%%", 100.0f * progress.converged); ImGui::NextColumn();

    ImGui::Text("Active blocks"); ImGui::NextColumn();
    ImGui::Text("%u", progress.activeBlocks); ImGui::NextColumn();

    ImGui::Separator();

    ImGui::Text("Delta time"); ImGui::NextColumn();
    ImGui::Text("%.2f ms", 1000.0 * mDeltaTime); ImGui::NextColumn();

#ifdef RT_ENABLE_INTERSECTION_COUNTERS
    const RayTracingCounters& counters = mViewport->GetCounters();
    ImGui::Separator();
    {
        ImGui::Text("Rays"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numRays / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Primary rays"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numPrimaryRays / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Shadow rays"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numShadowRays / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Shadow rays (hit)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numShadowRaysHit / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Ray-box tests (total)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numRayBoxTests / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Ray-box tests (passed)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numPassedRayBoxTests / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Ray-tri tests (total)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numRayTriangleTests / 1.0e+6f); ImGui::NextColumn();

        ImGui::Text("Ray-tri tests (passed)"); ImGui::NextColumn();
        ImGui::Text("%.3fM", (float)counters.numPassedRayTriangleTests / 1.0e+6f); ImGui::NextColumn();
    }
#endif // RT_ENABLE_INTERSECTION_COUNTERS

    ImGui::Columns(1);
}

static void ImGuiPrintTime(double t)
{
    if (t == 0)
    {
        ImGui::Text("0.0 s");
    }
    else if (t > 1.0)
    {
        ImGui::Text("%.4g s", t);
    }
    else if (t > 0.001)
    {
        ImGui::Text("%.4g ms", t * 1000.0);
    }
    else if (t > 0.000001)
    {
        ImGui::Text("%.4g us", t * 1000000.0);
    }
    else
    {
        ImGui::Text("%.4g ns", t * 1000000000.0);
    }
}

void DemoWindow::RenderUI_Profiler()
{
    static const char* selectedScope = nullptr;

    if (ImGui::Button("Reset"))
    {
        Profiler::GetInstance().ResetAll();
    }

    DynArray<ProfilerResult> profilerResults;
    Profiler::GetInstance().Collect(profilerResults);

    ImGui::Columns(4);

    ImGui::Text("Scope"); ImGui::NextColumn();
    ImGui::Text("Count"); ImGui::NextColumn();
    ImGui::Text("Avg. time"); ImGui::NextColumn();
    ImGui::Text("Min time"); ImGui::NextColumn();

    ImGui::Separator();

    for (const ProfilerResult& result : profilerResults)
    {
        bool selected = selectedScope == result.scopeName;
        if (ImGui::Selectable(result.scopeName, &selected))
        {
            selectedScope = result.scopeName;
        }
        ImGui::NextColumn();

        ImGui::Text("%lu", result.count); ImGui::NextColumn();
        ImGuiPrintTime(result.avgTime); ImGui::NextColumn();
        ImGuiPrintTime(result.minTime); ImGui::NextColumn();
    }

    ImGui::Columns(1);
}

void DemoWindow::RenderUI_Debugging()
{
    if (ImGui::TreeNode("Path"))
    {
        RenderUI_Debugging_Path();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Color"))
    {
        RenderUI_Debugging_Color();
        ImGui::TreePop();
    }

    if (ImGui::Button("Debug pixel"))
    {
        mPixelDebuggingPicking = true;
    }

    if (mPixelDebuggingPicking)
    {
        ImGui::SameLine();
        ImGui::Text("Picking...");
    }
}

void DemoWindow::RenderUI_Debugging_Path()
{
    for (uint32 i = 0; i < mPathDebugData.data.Size(); ++i)
    {
        ImGui::Separator();
        ImGui::Text("Ray #%u", i);

        ImGui::Columns(2);

        const RT::PathDebugData::HitPointData data = mPathDebugData.data[i];

        ImGui::Text("RayOrigin"); ImGui::NextColumn();
        ImGui::Text("[%f, %f, %f]", data.rayOrigin.x, data.rayDir.y, data.rayOrigin.z); ImGui::NextColumn();

        ImGui::Text("RayDir"); ImGui::NextColumn();
        ImGui::Text("[%f, %f, %f]", data.rayDir.x, data.rayDir.y, data.rayDir.z); ImGui::NextColumn();

        if (data.hitPoint.distance != FLT_MAX)
        {
            ImGui::Text("Distance"); ImGui::NextColumn();
            ImGui::Text("%f", data.hitPoint.distance); ImGui::NextColumn();

            ImGui::Text("Object ID"); ImGui::NextColumn();
            ImGui::Text("%u", data.hitPoint.objectId); ImGui::NextColumn();

            ImGui::Text("Sub Obj ID"); ImGui::NextColumn();
            ImGui::Text("%u", data.hitPoint.subObjectId); ImGui::NextColumn();

            ImGui::Text("Tri UV"); ImGui::NextColumn();
            ImGui::Text("[%f, %f]", data.hitPoint.u, data.hitPoint.v); ImGui::NextColumn();

            const Vec4f pos = data.shadingData.intersection.frame.GetTranslation();
            ImGui::Text("Position"); ImGui::NextColumn();
            ImGui::Text("[%f, %f, %f]", pos.x, pos.y, pos.z); ImGui::NextColumn();

            const Vec4f normal = data.shadingData.intersection.frame.GetTranslation();
            ImGui::Text("Normal"); ImGui::NextColumn();
            ImGui::Text("[%f, %f, %f]", normal.x, normal.y, normal.z); ImGui::NextColumn();

            //ImGui::Text("Tangent"); ImGui::NextColumn();
            //ImGui::Text("[%f, %f, %f]", data.shadingData.tangent.x, data.shadingData.tangent.y, data.shadingData.tangent.z); ImGui::NextColumn();

            //ImGui::Text("Tex coord"); ImGui::NextColumn();
            //ImGui::Text("[%f, %f]", data.shadingData.texCoord.x, data.shadingData.texCoord.y); ImGui::NextColumn();

            if (data.objectHit)
            {
                ImGui::Text("Object"); ImGui::NextColumn();
                if (ImGui::Button(data.objectHit->GetDynamicType()->GetName().Str()))
                {
                    mSelectedObject = const_cast<ISceneObject*>(data.objectHit);
                }
                ImGui::NextColumn();
            }

            if (data.shadingData.intersection.material)
            {
                ImGui::Text("Material"); ImGui::NextColumn();
                if (ImGui::Button(data.shadingData.intersection.material->debugName.Str()))
                {
                    mSelectedMaterial = const_cast<Material*>(data.shadingData.intersection.material);
                }
                ImGui::NextColumn();
            }

            ImGui::Text("Throughput"); ImGui::NextColumn();
#ifdef RT_ENABLE_SPECTRAL_RENDERING
            ImGui::Text("[%f, %f, %f, %f, %f, %f, %f, %f]",
                data.throughput.value[0], data.throughput.value[1], data.throughput.value[2], data.throughput.value[3],
                data.throughput.value[4], data.throughput.value[5], data.throughput.value[6], data.throughput.value[7]);
#else
            ImGui::Text("[%f, %f, %f]",
                data.throughput.value[0], data.throughput.value[1], data.throughput.value[2]);
#endif // RT_ENABLE_SPECTRAL_RENDERING
            ImGui::NextColumn();

            const char* bsdfEventStr = "Null";
            switch (data.bsdfEvent)
            {
            case BSDF::DiffuseReflectionEvent: bsdfEventStr = "Diffuse Reflection"; break;
            case BSDF::DiffuseTransmissionEvent: bsdfEventStr = "Diffuse Transmission"; break;
            case BSDF::GlossyReflectionEvent: bsdfEventStr = "Glossy Reflection"; break;
            case BSDF::GlossyRefractionEvent: bsdfEventStr = "Glossy Refraction"; break;
            case BSDF::SpecularReflectionEvent: bsdfEventStr = "Specular Reflection"; break;
            case BSDF::SpecularRefractionEvent: bsdfEventStr = "Specular Refraction"; break;
            }
            ImGui::Text("BSDF Event"); ImGui::NextColumn();
            ImGui::Text("%s", bsdfEventStr); ImGui::NextColumn();
        }

        ImGui::Columns(1);
    }

    const char* terminationReasonStr = "None";
    switch (mPathDebugData.terminationReason)
    {
    case PathTerminationReason::HitBackground: terminationReasonStr = "Hit background"; break;
    case PathTerminationReason::HitLight: terminationReasonStr = "Hit light"; break;
    case PathTerminationReason::Depth: terminationReasonStr = "Depth exeeded"; break;
    case PathTerminationReason::Throughput: terminationReasonStr = "Throughput too low"; break;
    case PathTerminationReason::NoSampledEvent: terminationReasonStr = "No sampled BSDF event"; break;
    case PathTerminationReason::RussianRoulette: terminationReasonStr = "Russian roulette"; break;
    }

    ImGui::Text("Path termination reason: %s", terminationReasonStr);
}

void DemoWindow::RenderUI_Debugging_Color()
{
    int32 x, y;
    GetMousePosition(x, y);

    uint32 width, height;
    GetSize(width, height);

    Vec4f hdrColor = Vec4f::Zero();
    Vec4f ldrColor = Vec4f::Zero();
    if (x >= 0 && y >= 0 && (uint32)x < width && (uint32)y < height)
    {
        // TODO this is incorrect, each pixel can have different number of samples
        const uint32 numSamples = mViewport->GetProgress().passesFinished;
        hdrColor = mViewport->GetSumBuffer().GetPixel(x, y) / static_cast<float>(numSamples);
        ldrColor = mViewport->GetFrontBuffer().GetPixel(x, y);
    }

    ImGui::Text("HDR color:");
#ifdef RT_ENABLE_SPECTRAL_RENDERING
    const Vec4f rgbHdrColor = RT::ConvertXYZtoRGB(hdrColor);
    ImGui::Text("  R: %f", rgbHdrColor.x);
    ImGui::Text("  G: %f", rgbHdrColor.y);
    ImGui::Text("  B: %f", rgbHdrColor.z);
    ImGui::Text("  X: %f", hdrColor.x);
    ImGui::Text("  Y: %f", hdrColor.y);
    ImGui::Text("  Z: %f", hdrColor.z);
#else
    ImGui::Text("  R: %f", hdrColor.x);
    ImGui::Text("  G: %f", hdrColor.y);
    ImGui::Text("  B: %f", hdrColor.z);
#endif // RT_ENABLE_SPECTRAL_RENDERING

    ImGui::Text("LDR color:");
    ImGui::Text("  R: %u", (uint32)(255.0f * ldrColor.x + 0.5f));
    ImGui::Text("  G: %u", (uint32)(255.0f * ldrColor.y + 0.5f));
    ImGui::Text("  B: %u", (uint32)(255.0f * ldrColor.z + 0.5f));

//#ifdef RT_ENABLE_SPECTRAL_RENDERING
    mSpectrumPicking = true;

    ImGui::PlotLines("Spectrum", mSpectrumDebugData.samples.Data(), mSpectrumDebugData.samples.Size(), 0, 0, FLT_MAX, FLT_MAX, ImVec2(0.0f, 100.0f));
//#endif
}

bool DemoWindow::RenderUI_Settings()
{
    bool resetFrame = false;

    //if (ImGui::TreeNode("Renderer"))
    //{
    //    resetFrame |= RenderUI_Settings_Rendering();
    //    ImGui::TreePop();
    //}

    if (EditObject("Renderer", mRenderer))
    {
        mViewport->SetRenderer(mRenderer.Get());
        resetFrame = true;
    }

    if (EditObject("Rendering Params", mRenderingParams))
    {
        resetFrame = true;
    }

    if (ImGui::TreeNode("Camera"))
    {
        resetFrame |= RenderUI_Settings_Camera();
        ImGui::TreePop();
    }

    if (EditObject("Postprocess Params", mPostprocessParams))
    {
        mViewport->SetPostprocessParams(mPostprocessParams);
    }

    if (mSelectedObject)
    {
        if (EditObject("Selection", *mSelectedObject))
        {
            resetFrame = true;

            // TODO not all changes require BVH rebuild
            mScene->BuildBVH();
        }
    }

    if (mSelectedMaterial)
    {
        if (EditObject("Material", *mSelectedMaterial))
        {
            resetFrame = true;
        }
    }

    // screenshot saving
    {
        if (ImGui::Button("LDR screenshot"))
        {
            mViewport->GetFrontBuffer().SaveBMP("screenshot.bmp", true);
        }

        ImGui::SameLine();

        if (ImGui::Button("HDR screenshot"))
        {
            // TODO this is incorrect
            const float colorScale = 1.0f / (float)mViewport->GetProgress().passesFinished;
            mViewport->GetSumBuffer().SaveEXR("screenshot.exr", colorScale);
        }
    }

    return resetFrame;
}

bool DemoWindow::RenderUI_Settings_Camera()
{
    bool resetFrame = false;

    if (ImGui::TreeNode("Transform"))
    {
        resetFrame |= ImGui::InputFloat3("Position", &mCameraSetup.position.x, 3);
        resetFrame |= EditEulerAngles("Orientation", mCameraSetup.orientation);

        resetFrame |= ImGui::InputFloat3("Velocity", &mCameraSetup.linearVelocity.x, 3);
        resetFrame |= ImGui::InputFloat3("Angular velocity", &mCameraSetup.angularVelocity.x, 3);

        ImGui::TreePop(); // Transform
    }

    if (ImGui::TreeNode("Lens"))
    {
        resetFrame |= ImGui::SliderFloat("Field of view", &mCameraSetup.fov, 0.5f, 120.0f);

        resetFrame |= ImGui::Checkbox("Enable DoF", &mCamera.mDOF.enable);
        resetFrame |= ImGui::SliderFloat("Aperture", &mCamera.mDOF.aperture, 0.0f, 10.0f, "%.3f", 5.0f);
        {
            ImGui::Columns(2, nullptr, false);
            resetFrame |= ImGui::SliderFloat("Focal distance", &mCamera.mDOF.focalPlaneDistance, 0.1f, 1000.0f, "%.3f", 2.0f);
            ImGui::NextColumn();
            if (ImGui::Button("Pick..."))
            {
                mFocalDistancePicking = true;
            }
            ImGui::Columns(1);
        }

        const char* bokehTypeNames[] = { "Circle", "Hexagon", "Box", "n-gon", "Texture" };
        int bokehTypeIndex = static_cast<int>(mCamera.mDOF.bokehShape);

        resetFrame |= ImGui::Combo("Bokeh Shape", &bokehTypeIndex, bokehTypeNames, (int)ArraySize(bokehTypeNames));
        if (mCamera.mDOF.bokehShape == BokehShape::NGon)
        {
            resetFrame |= ImGui::SliderInt("No. of blades", (int*)& mCamera.mDOF.apertureBlades, 3, 20);
        }

        resetFrame |= ImGui::Checkbox("Enable lens distortions", &mCamera.enableBarellDistortion);
        resetFrame |= ImGui::SliderFloat("Barrel distortion", &mCamera.barrelDistortionConstFactor, 0.0f, 0.2f);
        resetFrame |= ImGui::SliderFloat("Lens distortion", &mCamera.barrelDistortionVariableFactor, 0.0f, 0.2f);

        mCamera.mDOF.bokehShape = static_cast<BokehShape>(bokehTypeIndex);

        ImGui::TreePop(); // Lens
    }

    return resetFrame;
}

bool DemoWindow::RenderUI()
{
    bool resetFrame = false;

    uint32 width, height;
    GetSize(width, height);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = (float)mDeltaTime;
    io.KeyCtrl = IsKeyPressed(KeyCode::Control);
    io.KeyShift = IsKeyPressed(KeyCode::Shift);
    io.KeyAlt = IsKeyPressed(KeyCode::Alt);

    for (uint32 i = 0; i < 256; ++i)
    {
        io.KeysDown[i] = IsKeyPressed(static_cast<KeyCode>(i));
    }

    ImGui::NewFrame();
    {
        static bool showStats = true;
        static bool showProfiler = false;
        static bool showDebugging = false;
        static bool showRenderSettings = true;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::Checkbox("Settings", &showRenderSettings);
                ImGui::Checkbox("Debugging", &showDebugging);
                ImGui::Checkbox("Stats", &showStats);
                ImGui::Checkbox("Profiler", &showProfiler);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (showStats)
        {
            if (ImGui::Begin("Stats", &showStats))
            {
                RenderUI_Stats();
            }
            ImGui::End();
        }

        if (showProfiler)
        {
            if (ImGui::Begin("Profiler", &showProfiler))
            {
                RenderUI_Profiler();
            }
            ImGui::End();
        }

        if (showDebugging)
        {
            if (ImGui::Begin("Debugging", &showDebugging))
            {
                RenderUI_Debugging();
            }
            ImGui::End();
        }

        if (showRenderSettings)
        {
            if (ImGui::Begin("Settings", &showRenderSettings))
            {
                resetFrame = RenderUI_Settings();
            }
            ImGui::End();
        }
    }
    ImGui::EndFrame();

    ImGui::Render();

    memset(io.KeysDown, 0, sizeof(io.KeysDown));

    return resetFrame;
}

} // namesapce NFE