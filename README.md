nfEngine
========

The main purpose of this project is to create an open-source, multi-platform, efficient and universal 3D game engine. At the moment the engine is in early development state and is fully buildable on Windows platform only (with Direct3D 11 renderer). Detailed list of features can be found in a section below.

More information (features, detailed descriptions, contribution rules, etc.) can be found on our [Wiki page](https://github.com/nfprojects/nfengine/wiki).

Building the project - Windows
------------------------------

To make the code compilable, the following requirements have to be met:

1. Installed Visual Studio 2015 with Windows SDK.
2. Pulled external dependencies from [here](http://www.github.com/nfprojects/nfenginedeps)
    * **"nfEngineDeps"** directory will be created by using git submodules inside repo - fetch its contents by using `git submodule update --init` command
    * **NOTE:** Dependencies need to be built separately from engine. See README.md inside nfEngineDeps repo for more info.
3. Downloaded resources from [here](http://drive.google.com/open?id=0B66mya2agFOEd0RJUWx1aDZ6Ym8)
    * **"Data"** directory created in **"nfEngineDemo"** with content copied from subfolder **nfEngineTestData**
    * **NOTE:** it is convenient to use Google Drive application to synchronize resources automatically (by adding this folder to your drive) and create symbolic link to them.

Example Batch code on Windows:

```
cd nfengine/nfEngineDemo // go to root of repository
mklink /J Data "path-to-nfEngineTestData"
```

Building the project - Linux
----------------------------

Currently, buildable parts of nfEngine are: nfCommon, nfCommonTest, nfCommonPerfTest and nfRendererDemo. Requirements:

1. Installed a C++11-compatible compiler (right now the only compiler tested to work is GCC 4.7 and higher versions).
2. Installed CMake 2.6 or higher.
3. Pulled external dependencies from [here](http://www.github.com/nfprojects/nfenginedeps):
    * **"nfEngineDeps"** directory will be created by using git submodules inside repo - fetch its contents by using `git submodule update --init`
    * **NOTE:** Dependencies need to be built separately from engine. See README.md inside nfEngineDeps repo for more info.

To rebuild the entire project in one go, _Scripts/rebuild-all.sh_ script is available, which works similarly to "Batch Build" feature in Visual Studio. The script will call cleaning script and build the engine in all configurations possible. Unlike Windows platform, due to conflict between 32-bit and 64-bit versions of some packages, the Engine builds only in version which conforms Distros version.

Building without _Scripts/rebuild-all.sh_ script is done by explicitly calling CMake and Make. To build nfEngine with default settings (Release build, 64-bit platform), simply call on nfEngine repo root directory:

```
cmake .
make
```

CMake can be called only once, unless there is a need to recreate Makefiles, or to change build type/platform.

Changing build settings is done by defining CMake variables (all are optional - omitting one of them will use default setting mentioned above):
* **CMAKE_BUILD_TYPE** - specifies build type. Possible values: **Release**, **Debug**. Building nfEngine with **Debug** build type will turn off compiler optimization and generate debugging information for GDB.
* **SANITIZE** - specifies sanitizer to be used (by adding **-fsanitize=** compiler flag). Possible values: **thread**, **address**, **memory**, **undefined**. This requires support in a compiler (GCC >= 4.8 or clang).

Example - forcing Debug build:

```
cmake -DCMAKE_BUILD_TYPE=Debug .
make
```
