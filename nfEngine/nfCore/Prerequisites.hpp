/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  This file contains predeclarations of all needed clases.
 *         Including this file only instead of whole header can decrease compilation time.
 */

#pragma once

namespace NFE {

    namespace Common {
        class Window;
        class ThreadPool;
        enum class ImageFormat;
        class Image;
        class InputStream;
        class OutputStream;
    } // namespace Common

    namespace Util {
        class BVH;
        class FrameStats;
        class Aligned;
    } // namespace Util

    namespace Resource {
        class ResManager;
        class ResourceBase;
        class Material;
        class Mesh;
        class Texture;
        class CollisionShape;
        class SoundSample;
    } // namespace Resource

    namespace Scene {
        class Entity;
        class Component;
        class MeshComponent;
        class BodyComponent;
        class LightComponent;
        class Camera;

        class SceneManager;
        class PhysicsWorld;
        class Segment;
        struct SegmentLink;

        struct MeshComponentDesc;
        struct LightDesc;
        struct CameraDesc;
        struct BodyComponentDesc;

        enum class ComponentMsg;
        enum class ComponentType;
    } // namespace Scene

    namespace Renderer {
        class HighLevelRenderer;
        class DebugRenderer;
        class GBufferRenderer;
        class GuiRenderer;
        class LightsRenderer;
        class ShadowRenderer;
        class PostProcessRenderer;
        class RenderContext;
        class RenderTarget;
        class ShadowMap;
#define NFE_CONTEXT_ARG RenderContext *pContext

/// Structures passed to renderer modules
        struct RendererMaterial;
        struct CameraRenderDesc;
        class View;
    } // namespace Render

} // namespace NFE
