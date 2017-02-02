/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  This file contains forward declarations of all classes exposed be this project.
 *         Including this file only instead of whole header decreases compilation time.
 */

#pragma once

#include "nfCommon/Containers/UniquePtr.hpp"

#include <memory>


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
        class IEntityController;

        class IComponent;
        class MeshComponent;
        class BodyComponent;
        class LightComponent;
        class CameraComponent;
        class ControllerComponent;

        class SceneManager;
        class RendererSystem;
        class InputSystem;
        class EntitySystem;
        class EventSystem;
        class PhysicsSystem;

        class Event;
        class Event_Tick;
        class Event_Input;

        using EntityID = uint32;
        using EntityPtr = Common::UniquePtr<Entity>;
        using ComponentPtr = Common::UniquePtr<IComponent>;
        using EntityControllerPtr = Common::UniquePtr<IEntityController>;
        using SceneManagerPtr = Common::UniquePtr<SceneManager>;

    } // namespace Scene

    namespace Physics {
        struct BodyProxyInfo;
        class IPhysicsScene;
    } // namespace Physics

    namespace Renderer {
        class RenderScene;

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
