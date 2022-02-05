nfEngine
========

The main purpose of this project is to create an open-source, multi-platform, efficient and universal 3D game engine. At the moment the engine is in early development state. Detailed list of features can be found in a section below.

More information (features, detailed descriptions, contribution rules, etc.) can be found on our [Wiki page](https://github.com/nfprojects/nfengine/wiki).

Building the project - Windows
------------------------------

To prepare your build environment, following requirements have to be met:

1. Installed Visual Studio 2022 with Windows SDK.
2. (Optional) Installed Vulkan SDK (if you want to build Vulkan renderer; otherwise, it can be excluded from building)
3. Pulled external dependencies via Git submodules: `git submodule update --init --recursive`
4. (Optional) If Vulkan renderer is to be included, navigate to `Deps/glslang` directory and run `python update_glslang_sources.py`
5. CMake (3.21 or newer), with cmake.exe visible in PATH
6. msbuild.exe visible in PATH (for VS2022 Community it is typically located in
   `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`)

**Visual Studio build**

To build the project, load main solution file in Visual Studio and build it as normal - build system will
automatically pre-build dependencies on the first run.

**CMake build**

**NOTE:** This has only been tested using Visual Studio and might not work with other compilers (ex. MinGW).

There is an experimental build system available which allows to build the Engine using generated Visual Studio
project files on Windows. It was made mostly to accomodate coding with other editors with CMake integration plugins
(ex. Visual Studio Code) and could be used as a replacement for project-delivered Solution file for older Visual Studio
versions.

Keep in mind, however, that older versions of VS might not work - the Engine is often upgraded to latest version in order
to utilize new C++ features.

Building using Visual Studio Code and CMake Tools plugin was tested and should work out-of-the-box. However, if you prefer
to still use Visual Studio with this method, project files have to be manually generated via terminal or CMake GUI.

Out-of-source build is the only supported way to build the project. Remember to specify which generator and architecture
to use (only x64 builds are supported) - ex. in terminal (Git for Windows in this case):

```
mkdir build; cd build
cmake .. -A x64 -G "Visual Studio 17 2022"
```

This should create Visual Studio files which will compile both dependencies and the Engine. Build configuration (Debug or
Release) can be chosen inside Visual Studio files.


Building the project - Linux
----------------------------

Requirements:

1. Installed a C++17-compatible compiler (compiler tested to work is Clang 10 and GCC 10).
2. Installed CMake 3.18 or higher.
3. Installed Vulkan SDK
    * **NOTE:** Make sure that `VULKAN_SDK` env variable is set in the system and points to dir x86\_64 inside Vulkan SDK; ex. `export VULKAN_SDK=/home/user/VulkanSDK/bin/x86_64`
4. Pulled external dependencies via Git submodules: `git submodule update --init --recursive`
5. Installed libxcb and libxcb-image libraries and their headers

**NOTE:** Building Linux version of the Engine on Windows using WSL has been tested and works with WSLv2 -
WSL version 1 might work, but some tests will fail due to incompleteness of WSLv1.

Out-of-source CMake build is the only supported way to build the project:

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
