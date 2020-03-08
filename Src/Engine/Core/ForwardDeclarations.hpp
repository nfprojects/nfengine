/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  This file contains forward declarations of all classes exposed be this project.
 *         Including this file only instead of whole header decreases compilation time.
 */

#pragma once

#include "../Common/Containers/UniquePtr.hpp"

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
        class Material;
        class Mesh;
        class Texture;
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
        class TriggerComponent;

        class Scene;
        class RendererSystem;
        class InputSystem;
        class EntitySystem;
        class EventSystem;
        class PhysicsSystem;
        class TriggerSystem;

        class Event;
        class Event_Tick;
        class Event_Input;
        class Event_Trigger;

        using EntityID = uint32;
        using EntityPtr = Common::UniquePtr<Entity>;
        using ComponentPtr = Common::UniquePtr<IComponent>;
        using EntityControllerPtr = Common::UniquePtr<IEntityController>;
        using ScenePtr = Common::UniquePtr<Scene>;

    } // namespace Scene

} // namespace NFE
