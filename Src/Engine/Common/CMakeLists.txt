# @file
# @author Lookey (costyrra.xl@gmail.com)
# @brief  CMake for Common

MESSAGE(STATUS "Generating build files for Common")

SET(NFE_COMMON_DIRECTORY ${NFE_ENGINE_DIRECTORY}/Common)

# GLOB_RECURSE did not work on Windows, plus it's not recommended anyway
SET(NFCOMMON_SOURCES
    PCH.cpp
    nfCommon.cpp
    Config/Config.cpp
    Config/ConfigTokenizer.cpp
    Config/ConfigValue.cpp
    Containers/SharedPtrBase.cpp
    Containers/SharedPtrData.cpp
    Containers/String.cpp
    Containers/StringView.cpp
    FileSystem/FileAsync.cpp
    FileSystem/FileBuffered.cpp
    FileSystem/FileSystemCommon.cpp
    Image/Image.cpp
    Image/ImageBMP.cpp
    Image/ImageDDS.cpp
    Image/ImageFormat.cpp
    Image/ImageJPG.cpp
    Image/ImagePNG.cpp
    Image/ImageType.cpp
    Image/Mipmap.cpp
    Logger/Logger.cpp
    Logger/LoggerBackend.cpp
    Logger/LoggerImpl.cpp
    Logger/LogGrouper.cpp
    Logger/LogScope.cpp
    Logger/Backends/BackendConsole.cpp
    Logger/Backends/BackendHTML.cpp
    Logger/Backends/BackendTxt.cpp
    Logger/Backends/BackendXML.cpp
    Math/ColorHelpers.cpp
    Math/Conversions.cpp
    Math/Distribution.cpp
    Math/EquationSolver.cpp
    Math/Frustum.cpp
    Math/Geometry.cpp
    Math/HdrColor.cpp
    Math/HilbertCurve.cpp
    Math/LdrColor.cpp
    Math/Math.cpp
    Math/MathTypes.cpp
    Math/Matrix2.cpp
    Math/Matrix3.cpp
    Math/Matrix4.cpp
    Math/Packed.cpp
    Math/Quaternion.cpp
    Math/Random.cpp
    Math/SamplingHelpers.cpp
    Math/Transcendental.cpp
    Math/Transform.cpp
    Math/Utils.cpp
    Math/Vec4f.cpp
    Memory/Buffer.cpp
    Memory/DefaultAllocator.cpp
    Reflection/Object.cpp
    Reflection/ReflectionMember.cpp
    Reflection/ReflectionMemberMetadataBuilder.cpp
    Reflection/ReflectionTypeRegistry.cpp
    Reflection/ReflectionVariant.cpp
    Reflection/SerializationContext.cpp
    Reflection/Serializer.cpp
    Reflection/Types/ReflectionClassType.cpp
    Reflection/Types/ReflectionDynArrayType.cpp
    Reflection/Types/ReflectionEnumType.cpp
    Reflection/Types/ReflectionFundamentalType.cpp
    Reflection/Types/ReflectionNativeArrayType.cpp
    Reflection/Types/ReflectionPointerType.cpp
    Reflection/Types/ReflectionSharedPtrType.cpp
    Reflection/Types/ReflectionStringType.cpp
    Reflection/Types/ReflectionType.cpp
    Reflection/Types/ReflectionUniquePtrType.cpp
    System/Assertion.cpp
    System/SystemInfo.cpp
    Utils/BVH.cpp
    Utils/CompressedInt.cpp
    Utils/Entropy.cpp
    Utils/Latch.cpp
    Utils/MD5.cpp
    Utils/StringUtils.cpp
    Utils/TaskBuilder.cpp
    Utils/ThreadPool.cpp
    Utils/ThreadPoolTask.cpp
    Utils/Waitable.cpp
    Utils/Stream/BufferInputStream.cpp
    Utils/Stream/BufferOutputStream.cpp
    Utils/Stream/FileInputStream.cpp
    Utils/Stream/FileOutputStream.cpp
    Utils/Stream/InputStream.cpp
    Utils/Stream/OutputStream.cpp
)

SET(NFCOMMON_HEADERS
    Config/Config.hpp
    Config/ConfigCommon.hpp
    Config/ConfigDataTranslator.hpp
    Config/ConfigInterface.hpp
    Config/ConfigTokenizer.hpp
    Config/ConfigValue.hpp
    Containers/ArrayView.hpp
    Containers/ArrayViewImpl.hpp
    Containers/Comparator.hpp
    Containers/Deque.hpp
    Containers/DequeImpl.hpp
    Containers/DynArray.hpp
    Containers/DynArrayImpl.hpp
    Containers/FixedArray.hpp
    Containers/FixedArrayImpl.hpp
    Containers/Hash.hpp
    Containers/HashMap.hpp
    Containers/HashMapImpl.hpp
    Containers/HashSet.hpp
    Containers/HashSetImpl.hpp
    Containers/Iterators/ArrayIterator.hpp
    Containers/Iterators/ArrayIteratorImpl.hpp
    Containers/Map.hpp
    Containers/MapImpl.hpp
    Containers/PackedArray.hpp
    Containers/PackedArrayImpl.hpp
    Containers/Set.hpp
    Containers/SetImpl.hpp
    Containers/SharedPtr.hpp
    Containers/SharedPtrBase.hpp
    Containers/SharedPtrData.hpp
    Containers/SharedPtrImpl.hpp
    Containers/StaticArray.hpp
    Containers/StaticArrayImpl.hpp
    Containers/String.hpp
    Containers/StringImpl.hpp
    Containers/StringView.hpp
    Containers/StringViewImpl.hpp
    Containers/UniquePtr.hpp
    Containers/UniquePtrImpl.hpp
    Containers/WeakPtr.hpp
    Containers/WeakPtrImpl.hpp
    FileSystem/DirectoryWatch.hpp
    FileSystem/File.hpp
    FileSystem/FileAsync.hpp
    FileSystem/FileBuffered.hpp
    FileSystem/FileSystem.hpp
    ForwardDeclarations.hpp
    Image/Image.hpp
    Image/ImageBMP.hpp
    Image/ImageDDS.hpp
    Image/ImageFormat.hpp
    Image/ImageJPG.hpp
    Image/ImagePNG.hpp
    Image/ImageType.hpp
    Image/Mipmap.hpp
    Logger/Backends/BackendCommon.hpp
    Logger/Backends/BackendConsole.hpp
    Logger/Backends/BackendHTML.hpp
    Logger/Backends/BackendTxt.hpp
    Logger/Backends/BackendXML.hpp
    Logger/Backends/Windows/BackendWindowsDebugger.hpp
    Logger/Logger.hpp
    Logger/LoggerBackend.hpp
    Logger/LoggerImpl.hpp
    Logger/LogGrouper.hpp
    Logger/LogScope.hpp
    Math/Box.hpp
    Math/ColorHelpers.hpp
    Math/Constants.hpp
    Math/Conversions.hpp
    Math/Distribution.hpp
    Math/EquationSolver.hpp
    Math/Frustum.hpp
    Math/Geometry.hpp
    Math/Half.hpp
    Math/HalfImpl.hpp
    Math/HdrColor.hpp
    Math/HilbertCurve.hpp
    Math/LdrColor.hpp
    Math/Math.hpp
    Math/Matrix2.hpp
    Math/Matrix2Impl.hpp
    Math/Matrix3.hpp
    Math/Matrix3Impl.hpp
    Math/Matrix4.hpp
    Math/Packed.hpp
    Math/PackedLoadVec4f.hpp
    Math/PackedLoadVec8f.hpp
    Math/Plane.hpp
    Math/PlaneImpl.hpp
    Math/Quaternion.hpp
    Math/QuaternionImpl.hpp
    Math/Random.hpp
    Math/Ray.hpp
    Math/RayGeometry.hpp
    Math/Rectangle.hpp
    Math/SamplingHelpers.hpp
    Math/SimdBox.hpp
    Math/SimdGeometry.hpp
    Math/SimdRay.hpp
    Math/SimdTriangle.hpp
    Math/Sphere.hpp
    Math/SphericalQuad.hpp
    Math/Transcendental.hpp
    Math/TranscendentalImpl.hpp
    Math/Transform.hpp
    Math/Triangle.hpp
    Math/Utils.hpp
    Math/Vec16f.hpp
    Math/Vec16fImplAVX512.hpp
    Math/Vec16fImplNaive.hpp
    Math/Vec16i.hpp
    Math/Vec16iImplAVX512.hpp
    Math/Vec16iImplNaive.hpp
    Math/Vec2f.hpp
    Math/Vec2fImpl.hpp
    Math/Vec2x16f.hpp
    Math/Vec2x4f.hpp
    Math/Vec2x8f.hpp
    Math/Vec3f.hpp
    Math/Vec3fImpl.hpp
    Math/Vec3x16f.hpp
    Math/Vec3x4f.hpp
    Math/Vec3x8f.hpp
    Math/Vec4f.hpp
    Math/Vec4fImpl.hpp
    Math/Vec4fImplNaive.hpp
    Math/Vec4fImplSSE.hpp
    Math/Vec4fU.hpp
    Math/Vec4fUImpl.hpp
    Math/Vec4i.hpp
    Math/Vec4iImplNaive.hpp
    Math/Vec4iImplSSE.hpp
    Math/Vec8f.hpp
    Math/Vec8fImplAVX.hpp
    Math/Vec8fImplCommon.hpp
    Math/Vec8fImplNaive.hpp
    Math/Vec8i.hpp
    Math/Vec8iImplAVX2.hpp
    Math/Vec8iImplNaive.hpp
    Math/WindowFunctions.hpp
    Memory/Aligned.hpp
    Memory/Buffer.hpp
    Memory/DefaultAllocator.hpp
    Memory/MemoryHelpers.hpp
    nfCommon.hpp
    PCH.hpp
    Reflection/Object.hpp
    Reflection/ReflectionClassDeclare.hpp
    Reflection/ReflectionClassDefine.hpp
    Reflection/ReflectionEnumMacros.hpp
    Reflection/ReflectionMember.hpp
    Reflection/ReflectionMemberMetadataBuilder.hpp
    Reflection/ReflectionMemberPath.hpp
    Reflection/ReflectionTypeRegistry.hpp
    Reflection/ReflectionTypeResolver.hpp
    Reflection/ReflectionUnitTestHelper.hpp
    Reflection/ReflectionUtils.hpp
    Reflection/ReflectionVariant.hpp
    Reflection/SerializationContext.hpp
    Reflection/Serializer.hpp
    Reflection/Types/ReflectionArrayType.hpp
    Reflection/Types/ReflectionClassType.hpp
    Reflection/Types/ReflectionDynArrayType.hpp
    Reflection/Types/ReflectionEnumType.hpp
    Reflection/Types/ReflectionFundamentalType.hpp
    Reflection/Types/ReflectionNativeArrayType.hpp
    Reflection/Types/ReflectionPointerType.hpp
    Reflection/Types/ReflectionSharedPtrType.hpp
    Reflection/Types/ReflectionStringType.hpp
    Reflection/Types/ReflectionType.hpp
    Reflection/Types/ReflectionUniquePtrType.hpp
    System/Assertion.hpp
    System/AsyncQueueManager.hpp
    System/ConditionVariable.hpp
    System/Console.hpp
    System/KeyCodes.hpp
    System/Library.hpp
    System/Memory.hpp
    System/Mutex.hpp
    System/RWLock.hpp
    System/RWSpinLock.hpp
    System/RWSpinLockImpl.hpp
    System/SpinLock.hpp
    System/SpinLockImpl.hpp
    System/SystemInfo.hpp
    System/SystemInfoConstants.hpp
    System/Thread.hpp
    System/Timer.hpp
    System/Window.hpp
    Utils/BitUtils.hpp
    Utils/BitUtilsImplCommon.hpp
    Utils/BitUtilsImplLinuxAVX.hpp
    Utils/BitUtilsImplNative.hpp
    Utils/BitUtilsImplWindowsAVX.hpp
    Utils/BVH.hpp
    Utils/CompressedInt.hpp
    Utils/Entropy.hpp
    Utils/EnumIterator.hpp
    Utils/FundamentalTypesUnion.hpp
    Utils/LanguageUtils.hpp
    Utils/Latch.hpp
    Utils/MD5.hpp
    Utils/ParallelAlgorithms.hpp
    Utils/ScopedLock.hpp
    Utils/Stream/BufferInputStream.hpp
    Utils/Stream/BufferOutputStream.hpp
    Utils/Stream/FileInputStream.hpp
    Utils/Stream/FileOutputStream.hpp
    Utils/Stream/InputStream.hpp
    Utils/Stream/OutputStream.hpp
    Utils/Stream/StreamCommon.hpp
    Utils/StringUtils.hpp
    Utils/TaskBuilder.hpp
    Utils/ThreadPool.hpp
    Utils/ThreadPoolTask.hpp
    Utils/Waitable.hpp
)

SET(NFCOMMON_NATVIS
    Containers/Containers.natvis
    Math/Math.natvis
)

IF(WIN32)
    # Windows-specific files
    SET(NFCOMMON_SOURCES ${NFCOMMON_SOURCES}
        FileSystem/Windows/DirectoryWatch.cpp
        FileSystem/Windows/File.cpp
        FileSystem/Windows/FileAsyncPlatform.cpp
        FileSystem/Windows/FileSystem.cpp
        Logger/Backends/Windows/BackendWindowsDebugger.cpp
        System/Windows/AssertionWindows.cpp
        System/Windows/AsyncQueueManager.cpp
        System/Windows/Common.cpp
        System/Windows/Console.cpp
        System/Windows/Library.cpp
        System/Windows/Main.cpp
        System/Windows/Memory.cpp
        System/Windows/SystemInfoPlatform.cpp
        System/Windows/Thread.cpp
        System/Windows/Timer.cpp
        System/Windows/Window.cpp
    )

    SET(NFCOMMON_HEADERS ${NFCOMMON_HEADERS}
        System/Windows/AsyncQueueManager.hpp
        System/Windows/Common.hpp
        System/Windows/ConditionVariableImpl.hpp
        System/Windows/Console.hpp
        System/Windows/MutexImpl.hpp
        System/Windows/RWLockImpl.hpp
    )
ELSEIF(UNIX)
    # Linux-specific files
    SET(NFCOMMON_SOURCES ${NFCOMMON_SOURCES}
        FileSystem/Linux/DirectoryWatch.cpp
        FileSystem/Linux/File.cpp
        FileSystem/Linux/FileAsyncPlatform.cpp
        FileSystem/Linux/FileSystem.cpp
        System/Linux/AssertionLinux.cpp
        System/Linux/AsyncQueueManager.cpp
        System/Linux/Console.cpp
        System/Linux/Library.cpp
        System/Linux/Memory.cpp
        System/Linux/SystemInfoPlatform.cpp
        System/Linux/Thread.cpp
        System/Linux/Timer.cpp
        System/Linux/Window.cpp
    )

    SET(NFCOMMON_HEADERS ${NFCOMMON_HEADERS}
        System/Linux/AsyncQueueManager.hpp
        System/Linux/ConditionVariableImpl.hpp
        System/Linux/Console.hpp
        System/Linux/MutexImpl.hpp
        System/Linux/RWLockImpl.hpp
    )
ELSE(WIN32)
    MESSAGE(FATAL_ERROR "Target platform not supported")
ENDIF(WIN32)

IF(UNIX)
    # Search for dependencies
    PKG_CHECK_MODULES(NFE_COMMON_DEPS REQUIRED xcb xcb-image)
ENDIF(UNIX)

# setup directories
ADD_LIBRARY(Common SHARED ${NFCOMMON_SOURCES} ${NFCOMMON_HEADERS} ${NFCOMMON_NATVIS})

TARGET_COMPILE_DEFINITIONS(Common
    PRIVATE NFCOMMON_EXPORTS
)

TARGET_INCLUDE_DIRECTORIES(Common
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}
    PRIVATE ${NFEDEPS_ROOT_DIRECTORY}/nfenginedeps
    PRIVATE ${NFE_COMMON_DIRECTORY}
    PRIVATE ${NFE_BUILD_DIRECTORY}/Deps/libpng
    PRIVATE ${NFE_BUILD_DIRECTORY}/Deps/zlib
)

TARGET_LINK_DIRECTORIES(Common
    PRIVATE ${NFEDEPS_LIB_DIRECTORY}
)

ADD_DEPENDENCIES(Common NFEDepsPostBuild)
TARGET_PRECOMPILE_HEADERS(Common PRIVATE PCH.hpp)

IF(WIN32)
    SET(NFE_COMMON_SYSTEM_DEPS dbghelp)
ELSEIF(UNIX)
    SET(NFE_COMMON_SYSTEM_DEPS dl)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(Common squish jpeg png_static ${NFE_COMMON_SYSTEM_DEPS} ${NFE_COMMON_DEPS_LIBRARIES})

SET_PROPERTY(TARGET Common PROPERTY FOLDER Src/Engine)
NFE_SOURCE_GROUP_BY_DIR(NFCOMMON_SOURCES)
NFE_SOURCE_GROUP_BY_DIR(NFCOMMON_HEADERS)
NFE_SOURCE_GROUP_PCH(Common)
