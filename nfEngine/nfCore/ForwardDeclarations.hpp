/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  This file contains forward declarations of all classes exposed be this project.
 *         Including this file only instead of whole header decreases compilation time.
 */

#pragma once

namespace NFE {

    class Engine;

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
        class Component;
        class MeshComponent;
        class BodyComponent;
        class LightComponent;
        class CameraComponent;
        class TransformComponent;

        class EntityManager;
        class SceneManager;
        class TransformSystem;
        class PhysicsSystem;
        class RendererSystem;

        class RenderingData;
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
        class GeometryRenderer;
        class GuiRenderer;
        class LightsRenderer;
        class PostProcessRenderer;

        struct GeometryRendererContext;
        struct LightsRendererContext;
        struct PostProcessRendererContext;
        struct GuiRendererContext;
        struct DebugRendererContext;

        class RenderContext;
        class RenderTarget;
        class ShadowMap;

        /// Structures passed to renderer modules
        struct RendererMaterial;
        struct CameraRenderDesc;
        class View;
        class Font;
    } // namespace Renderer

} // namespace NFE
