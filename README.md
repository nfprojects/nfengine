nfEngine
========

The main purpose of this project is to create an open-source, multi-platform, efficient and universal 3D game engine. At the moment the engine is in early development state. Detailed list of features can be found in a section below.

More information (features, detailed descriptions, contribution rules, etc.) can be found on our [Wiki page](https://github.com/nfprojects/nfengine/wiki).

Building the project - Windows
------------------------------

To make the code compilable, the following requirements have to be met:

1. Installed Visual Studio 2019 with Windows SDK.
2. Installed Vulkan SDK (if you want to build Vulkan renderer; otherwise, it can be excluded from building)
3. Pulled external dependencies from [here](http://www.github.com/nfprojects/nfenginedeps)
    * **"nfEngineDeps"** directory will be created by using git submodules inside repo - fetch its contents by using `git submodule update --init` command
    * **NOTE:** Dependencies need to be built separately from engine. See README.md inside nfEngineDeps repo for more info.


Building the project - Linux
----------------------------

Currently buildable parts of nfEngine are: Common, CommonTest, CommonPerfTest, RendererVk and RendererDemo.
Requirements:

1. Installed a C++17-compatible compiler (compiler tested to work is Clang 10 and GCC 10).
2. Installed CMake 3.18 or higher.
3. Installed Vulkan SDK
    * **NOTE:** Make sure that `VULKAN_SDK` env variable is set in the system and points to dir x86\_64 inside Vulkan SDK.
4. Pulled external dependencies from [here](http://www.github.com/nfprojects/nfenginedeps):
    * **"nfEngineDeps"** directory will be created by using git submodules inside repo - fetch its contents by using `git submodule update --init`
    * **NOTE:** Dependencies need to be built separately from engine. See README.md inside nfEngineDeps repo for more info.

To rebuild the entire project in one go, _Scripts/rebuild-all.sh_ script is available, which works similarly to "Batch Build" feature in Visual Studio. The script will call cleaning script and build the engine in all configurations possible. Unlike Windows platform, due to conflicts between 32-bit and 64-bit versions of some packages, the Engine builds only in version which conforms Distros version.

If you need to manually build (without cleaning) single configuration, _Scripts/build.sh_ script can be used. It's only argument is a build configuration and can be **Debug** or **Release**. Calling without argument assumes Release build.

Building without _Scripts/rebuild-all.sh_ script is done by explicitly calling CMake and Make. To build nfEngine with default settings (Release build), simply call on nfEngine repo root directory:

```
cmake .
make
```

CMake can be called only once, unless there is a need to recreate Makefiles, or to change build type/platform.

Changing build settings is done by defining CMake variables (all are optional - omitting one of them will use default setting mentioned above):
* **CMAKE_BUILD_TYPE** - specifies build type. Possible values: **Release**, **Debug**. Building nfEngine with **Debug** build type will turn off compiler optimization and generate debugging information for GDB.
* **SANITIZE** - specifies sanitizer to be used (by adding **-fsanitize=** compiler flag). Possible values: **thread**, **address**, **memory**, **undefined**. This requires support in a compiler.

Example - forcing Debug build:

```
cmake -DCMAKE_BUILD_TYPE=Debug .
make
```
