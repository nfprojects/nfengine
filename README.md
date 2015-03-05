nfEngine
========

The main purpose of this project is to create an open-source, multi-platform, efficient and universal 3D game engine. At the moment the engine is in early development state and supports Windows platform only (with Direct3D 11 renderer). Detailed list of features can be found in a section below.

Features
--------

**Note:** the features marked in *italics* are planned.

**General:**

* Written in C++
* Portability: Windows, *Linux, MacOS*
* Verbose logging
* Custom math library based on SSE
* *Advanced scripting (deep configuration, scene modification, resources generation, etc.)*

**Resources:**

* Multithreaded, asynchronous loading
* Virtual file system
* Supported resource types: Texture, Material, Mesh, Collision shape
* *Resource file modification detection*
* *Memory usage control, garbage collector*

**Scene management:**

* Infinite worlds
* Component-entity design
* Supported components: Mesh, Light, Body, Camera, *SoundSource, Particle Emitter*
* Scene serialization
* Rigid body physics
* *Multithreaded streaming & procedural generation*

**Rendering:**

* Switchable renderers: Direct3D 11, *OpenGL 4.4*
* Multithreaded pipeline
* Deferred shading
* Dynamic lights and shadows
* Advanced postprocess

Directory structure
-------------------

The engine's root directory looks as follows:

* **Bin** - compiled binaries
* **Logs** - various logs
* **nfEngine** - main engine's code directory
    * **nfCommon** - common utilities library that could be used outside the engine
    * **nfCommonTest** - unit tests for nfCommon
    * **nfCore** - the engine's core
    * **nfCoreTest** - unit tests for nfCore
    * **nfRenderer** - renderer (Direct3D 11 implementation, legacy)
        * **Shaders** - HLSL shaders for Direct3D 11 renderer
    * **Tools**
        * **CollisionShapeConverter** - OBJ to engine's physics collision shape format converter
        * **FastMeshConverter** - OBJ to engine's model format converter
        * **PackerTool** - command line utility for PAK files creation
    * **Renderers**
        * **RendererInterface**
        * **Shaders** 
* **nfEngineDeps** - engine's dependencies directory. See "Dependencies" section for more information
* **nfEngineTest** - demo application
* **Obj** - temporary compilation objects
* **Scripts** - miscellaneous batch and bash scripts
* **ShaderCache** and **ShaderCache_Debug** - compiled shaders

Dependencies
------------

The engine is dependent on following libraries:

- [AntTweakBar](http://anttweakbar.sourceforge.net/doc/) - simple, temporary GUI
- [Bullet Physics Library](http://bulletphysics.org/wordpress/) - physics engine
- [FreeType](http://www.freetype.org/freetype2/) - fonts rendering
- [libpng](http://www.libpng.org/pub/png/libpng.html) - PNG files decoding
- [jpgd](https://code.google.com/p/jpgd/) - JPEG decoding
- [RapidXml](http://rapidxml.sourceforge.net/) - XML parsing
- [rapidjson](https://code.google.com/p/rapidjson/) - JSON parsing
- [zlib](http://www.zlib.net/) - libpng and FreeType dependency, will be used to compress/decompress files in the future
- [model_obj](http://www.dhpoware.com/demos/glObjViewer.html) - OBJ model files parsing
- [Google Test](https://code.google.com/p/googletest/) - unit tests framework

To make the code compileable, the following requirements have to be met:

1. Installed Visual Studio 2013.
2. Installed DirectX SDK.
3. Pulled external dependencies from [here](http://www.github.com/nfprojects/nfenginedeps)
    * **"nfEngineDeps"** directory created by using git submodules inside repo
    * **NOTE:** Dependencies need to be built separately from engine. See README.md inside nfEngineDeps repo for more info.
4. Downloaded resources from [here](http://drive.google.com/open?id=0B66mya2agFOEd0RJUWx1aDZ6Ym8)
    * **"Data"** directory created in **"nfEngineTest"** with content copied from subfolder **nfEngineTestData**
    * **NOTE:** it is convenient to use Google Drive application to synchronize resources automatically (by adding this folder to your drive) and create symbolic link to them:
            - on Windows: "mklink /J <dest> <src>"
            - on Linux: "ln -s <src> <dest>"

Example code on Windows:

```
cd nfengine/nfEngineTest // go to root of repository
mklink /J Data "path-to-nfEngineTestData"
```

Documentation
-------------

Doxygen documentation can be generated using "gen_doc.bat" script. Doxygen needs to be installed in the system and "doxygen" command must be visible in the shell.

Formatting
----------

Keep the code format consistent. Run format.bat script to format the code automatically. ArtisticStyle needs to be installed in the system and "astyle" command must be visible in the shell.

ArtisticStyle can be downloaded [here](http://astyle.sourceforge.net/).

Basic coding guidelines:

* Code is written in [Allman style](http://en.wikipedia.org/wiki/Indent_style#Allman_style).
* We use spaces to indent parts of code. Each indentation takes 4 spaces. Mixed tabs and spaces are forbidden.
* To make code reviewing easier, keep max 100 chars per line. Exceptions to that rule might occur if it is absolutely necessary, depending on the situation (keep in mind, some legacy code in the repository might not follow this rule).
* Each source file should have a doxygen boilerplate which will briefly describe what this module does. This rule applies to header files as well.
* Use two newlines to separate chunks of code in one file (e.g. to split constants from function/method definitions, to split boilerplate from rest of the file, or to split namespaces).
* Naming rules:
    * **camelCase**, e.g.: ``int fontSize;``
    * **Begin with lower case:** local variables, public class members, function arguments.
    * **Begin with Upper case:** namespace names, class/structure/union/enum names, function/method names.
    * Add "m" prefix before private and protected class member names, e.g.: ``int mWidth;``.
    * Add "g" prefix before global variable names.
    * Write preprocessor macros using upper case only, e.g.: ``#define LOG_ERROR(x) ...``.
    * Don't use Hungarian notation!
* File naming rules:
    * Begin with upper case.
    * Use proper extension: *cpp* if it's C++ source file, *hpp* if it's C++ header, etc.
* Miscellaneous:
    * Write comments using [Doxygen](http://www.stack.nl/~dimitri/doxygen/manual/docblocks.html) style.

Issue tracking
--------------

Working on nfEngine revolves around GitHub and its issue tracking system. You can find all current issues at [our GitHub Issue Tracker](https://github.com/nfprojects/nfengine/issues).

Usually issues are added by owners and categorized according with three labels:

1. **Type of issue:**
    * bug
    * enhancement
    * new feature
2. **Priority (how important is this issue for the project right now):**
    * low priority
    * medium priority
    * high priority
3. **Estimated complexity of the issue:**
    * small
    * medium
    * huge

Additional rules applied to labelling issues:

1. Issues labeled as **"new feature"** or **"enhancement"** label can additionally have a **"proposal"** label. Such issues are most probably not completely defined, might be a subject to change and should not be assigned to anyone until **"proposal"** label disappears. Removal of **"proposal"** will happen only after a discussion with project owners, or after completion of issue.
2. Issues labeled with **"bug"** label don't need to be tagged with estimate label if reporter cannot perform such estimation.

When a bug is found, it should be reported on issue tracking system with only **"bug"** label. Such issue should contain detailed information on how to reproduce the bug. After verification of the issue, one of the owners will further process the issue (either by prioritizing it and assigning it to someone, or by closing it if issue is invalid).

Issues crucial to project, usually related to one specific goal, are categorized into milestones. Milestones don't have specific due date. Issues not assigned to milestones are mostly general, small issues (most of these are probably bugs).

Workflow and branching
----------------------

nfEngine project contains two main long-running branches with code:

* **Branch "master"** - here is kept latest stable version of nfEngine, guaranteed to work. On this branch no new changes are committed - it should contain only merge commits with version change.
* **Branch "devel"** - this branch is main development branch used to contribute new changes.

Since the end of October, review of nfEngine commits is moved to GerritHub. All review should be done there - pull requests coming from GitHub will be automatically rejected.

Change will be merged only when it will pass code review and verification process, and when provided change will be mergeable to devel branch without conflicts. Before committing it is recommended to use pre-commit script hidden in Scripts directory, which will automatically run cppcheck, astyle and tests. As a point of reference, use this script on current top commit on devel branch - same results between run on remote branch devel and on change to be pushed will almost guarantee passed Verification process.

Changes to master branch are forbidden and will be rejected immediately by project owners.

Large changes (more than 500 lines affected) should be split in multiple commits to make review process easier.


Commit message format
---------------------

Commit messages should keep a simple, standard format. Title (first line of commit message) should describe briefly what change does. Then contributor should write more detailed description of committed change and way to verify provided change. Example commit message is provided below:

```
Fix memory leaks in nfCommon Math module

This commit fixes memory leaks which occurred when using NFE::Common::Vector. Leaks
occurred due to not freed memory in Vector destructor.

Verification:
Build, run tests using valgrind - no leaks should occur.
```

If needed, change can be split into multiple sub-changes. These changes should depend on each other and appropriate information should be provided in commit message with [PATCH x/y] tag.
