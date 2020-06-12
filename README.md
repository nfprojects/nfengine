nfEngine
========

The main purpose of this project is to create an open-source, multi-platform, efficient and universal 3D game engine. At the moment the engine is in early development state. Detailed list of features can be found in a section below.

More information (features, detailed descriptions, contribution rules, etc.) can be found on our [Wiki page](https://github.com/nfprojects/nfengine/wiki).

Building the project - Windows
------------------------------

To make the code compilable, the following requirements have to be met:

1. Installed Visual Studio 2019 with Windows SDK.
2. Installed Vulkan SDK (if you want to build Vulkan renderer; otherwise, it can be excluded from building)
3. Pulled external dependencies via Git submodules: `git submodule update --init --recursive`
4. CMake, with cmake.exe visible in PATH
5. msbuild.exe visible in PATH (for VS2019 Community it is typically located in
   `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin`)


Building the project - Linux
----------------------------

Currently buildable parts of nfEngine are: Common, CommonTest, CommonPerfTest, RendererVk and RendererDemo.
Requirements:

1. Installed a C++17-compatible compiler (compiler tested to work is Clang 10 and GCC 10).
2. Installed CMake 3.18 or higher.
3. Installed Vulkan SDK
    * **NOTE:** Make sure that `VULKAN_SDK` env variable is set in the system and points to dir x86\_64 inside Vulkan SDK.
4. Pulled external dependencies via Git submodules: `git submodule update --init --recursive`
5. Installed libxcb and libxcb-image libraries and their headers

Out-of-source CMake build is recommended to make the cleaning process easier and not make the repo itself dirty, ex.

```
mkdir build; cd build
cmake ..
make
```

Changing build settings is done by defining CMake variables (all are optional):
* **CMAKE_BUILD_TYPE** - specifies build type. Possible values: **Release**, **Debug**. Building nfEngine with **Debug** build type will turn off compiler optimization and generate debugging information for GDB.
* **SANITIZE** - specifies sanitizer to be used (by adding **-fsanitize=** compiler flag). Possible values: **thread**, **address**, **memory**, **undefined**. This requires support in a compiler.

To rebuild the entire project in one go, _Scripts/rebuild-all.sh_ script is available, which works similarly to "Batch Build" feature in Visual Studio. The script will call cleaning script and build the engine in all configurations possible. Unlike Windows platform, due to conflicts between 32-bit and 64-bit versions of some packages, the Engine builds only in version which conforms Distros version.

If you need to manually build (without cleaning) single configuration, _Scripts/build.sh_ script can be used. It's only argument is build configuration and can be **Debug** or **Release**. Calling without argument assumes Release build.
