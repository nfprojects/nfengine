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
        class GameObjectInstance;

        class IComponent;
        class MeshComponent;
        class BodyComponent;
        class LightComponent;
        class CameraComponent;
        class ScriptComponent;

        class SceneManager;
        class IPhysicsSystem;
        class RendererSystem;
        class InputSystem;
        class EntitySystem;
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
