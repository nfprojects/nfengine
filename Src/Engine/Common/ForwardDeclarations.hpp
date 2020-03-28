/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  This file contains forward declarations of all classes exposed be this project.
 *         Including this file only instead of whole header decreases compilation time.
 */

#pragma once

namespace NFE {

namespace Common {

// Containers
class String;
class StringView;

// Config
class Config;
class ConfigObject;
class ConfigArray;
class ConfigValue;
class ConfigTokenizer;
struct Token;
template <class T> class DataTranslator;

// FileSystem
enum class SeekMode;
enum class AccessMode;
enum class PathType;
class File;
class FileAsync;
class FileBuffered;
class FileSystem;
class DirectoryWatch;

// Image
enum class MipmapFilter;
enum class ImageFormat;
class Image;
class Mipmap;
class ImageType;
class InputStream;
class OutputStream;

// Logger
enum class LogType;
class ILoggerBackend;
class Logger;
class LogGrouper;
class LogScope;

// Memory
class Buffer;
class DefaultAllocator;
struct AllocationDebugInfo;
struct AllocatorStats;

// Utils
class InputStream;
class FileInputStream;
class BufferInputStream;
class OutputStream;
class FileOutputStream;
class BufferOutputStream;
class AsyncThreadPool;
using AsyncFuncID = uint64_t;
struct BVHNode;
struct BVHStats;
class BVH;
class Task;
class WorkerThread;
class ThreadPool;
struct TaskContext;
using TaskID = uint32;
class Latch;
class Waitable;
class TaskBuilder;

// System
enum class KeyCode : unsigned int;
class AsyncQueueManager;
class Library;
class SystemInfo;
class Timer;
class Window;
class Thread;
enum class ThreadPriority;

} // namespace Common


namespace Math {

enum class IntersectionResult;
struct Vec2f;
struct Vec3f;
struct Vec4fU;
struct VecBool4f;
struct VecBool4i;
struct VecBool8f;
struct VecBool8i;
struct Vec4i;
struct Vec8i;
struct Vec4f;
struct Vec8f;
class Vec3x8f;
class Vec2x8f;
struct Quaternion;
class Transform;
class Triangle;
class Sphere;
class Ray;
class Frustum;
class Box;
class RayBoxSegment;
class Random;
class Matrix2;
class Matrix3;
class Matrix4;
class Triangle_Simd8;
class Ray_Simd8;
class Box_Simd8;
class Distribution;
class Half;
struct Half2;
struct Half3;
struct Half4;
class PackedUnitVector3;
class PackedColorRgbHdr;
class SharedExpFloat3;
class PackedFloat3;
struct Packed565;
class LdrColorRGB;
class LdrColorRGBA;
class HdrColorRGB;

} // namespace Math


namespace RTTI {

enum class TypeKind : uint8;
struct TypeInfo;
struct ClassTypeInfo;
class Type;
class TypeRegistry;
class Member;
class ITypeSerializer;
template<typename T> class TypeCreator;

} // namespace RTTI

} // namespace NFE
