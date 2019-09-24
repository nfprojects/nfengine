#pragma once

#include <memory>

namespace NFE {
namespace RT {

class Bitmap;

struct HitPoint;
struct IntersectionData;
struct SingleTraversalContext;
struct PacketTraversalContext;
struct RenderingContext;
struct ShadingData;
struct PathDebugData;
class Film;
struct RayPacket;

class Camera;
class Scene;

class IShape;
class ILight;

class ISceneObject;
class ITraceableSceneObject;
class LightSceneObject;
class ShapeSceneObject;
class DecalSceneObject;

class IRenderer;

class IRendererContext;

class Material;

class ITexture;

} // namespace RT
} // namespace NFE
