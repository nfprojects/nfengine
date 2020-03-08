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
struct RenderingParams;
struct ShadingData;
struct PathDebugData;
class Film;
struct RayPacket;

class Camera;
class Scene;

class IShape;
class ILight;
class IMedium;

class ISceneObject;
class ITraceableSceneObject;
class ShapeSceneObject;
class LightSceneObject;
class DecalSceneObject;

class IRenderer;

class IRendererContext;

class BSDF;
class Material;

class ITexture;

struct Wavelength;
struct RayColor;
class IColor;

} // namespace RT
} // namespace NFE
