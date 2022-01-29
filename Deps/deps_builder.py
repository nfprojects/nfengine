#!/usr/bin/env python

import argparse
import itertools
import os
import pathlib
import platform
import shutil
import signal
import subprocess
import sys
import threading
import time
import datetime


def SecondsToHumanReadableString(time):
    min, sec = divmod(int(time), 60)
    if sec < 10:
        return str(min) + "m 0" + str(sec) + "s"
    else:
        return str(min) + "m " + str(sec) + "s"


class DepsBuilder:
    def __init__(self, projectName, generator, requirements, deps, platforms, configurations):
        signal.signal(signal.SIGINT, self.InterruptHandler)

        self.mRequirements = requirements
        self.mProjectName = projectName
        self.mGenerator = generator
        self.mDeps = deps
        self.mPlatforms = platforms
        self.mConfigs = configurations
        self.mAnimThread = None
        self.mAnimDone = False
        self.mBuildAllPlats = False
        self.mBuildAllConfigs = False
        self.mThreads = 0

        if platforms is None:
            self.mPlatforms = ["x64"]

        if configurations is None:
            self.mConfigs = ["Release"]

        self.mScriptDir = pathlib.PurePath(os.path.dirname(os.path.realpath(sys.argv[0])))
        self.mBuildDir = self.mScriptDir / 'build'
        self.mBinDir = self.mScriptDir / 'Bin'

        print("=== " + self.mProjectName + " dependency builder ===\n")

        if platform.system() != "Windows":
            print("This script is designed to work under Windows to help build dependencies for MSVC.")
            print("Linux builds directly include CMake projects, so this script is not necessary. In")
            print("order to build dependencies on Linux, just run CMake on the project and everything")
            print("will work automagically.\n")
            sys.exit(1)

        parser = argparse.ArgumentParser(description="Dependency builder script for " + self.mProjectName + ".")
        parser.add_argument('-p', '--platform', dest='plat', nargs='?', default="",
                            help="Specify for which platform CMake should generate project files.")
        parser.add_argument('-c', '--configuration', dest='config', nargs='?', default="",
                            help="Specify which configuration the build should use.")
        parser.add_argument('-v', '--verbose', dest='verbose', action="store_true",
                            help="Print extra information (CMake output, MSBuild output)")
        parser.add_argument('--clean', dest='clean', action="store_true",
                            help="Clean given platform/configuration.")
        parser.add_argument('--noanim', dest='noanim', action="store_true",
                            help="Disable progress animation (useful for non-terminal enviroments, ex. VS Output window)")
        parser.add_argument('-t', '--threads', dest='threads', nargs='?', default="",
                            help="Build on multiple threads. Leave empty to use max CPU count.")

        try:
            self.mArgs = parser.parse_args()
        except Exception as e:
            print("Failed to parse arguments: " + e)
            sys.exit(1)

        if not self.mArgs.plat:
            print("Platform not specified. Building all possible platforms.")
            self.mBuildAllPlats = True
        elif self.mArgs.plat not in self.mPlatforms:
            print("Invalid platform provided: " + self.mArgs.plat)
            print("Available configurations are:")
            for p in self.mPlatforms:
                print("    " + p)
            print('\n')
            sys.exit(1)

        if not self.mArgs.config:
            print("Configuration not specified. Building all possible configurations.")
            self.mBuildAllConfigs = True
        elif self.mArgs.config not in self.mConfigs:
            print("Invalid configuration provided: " + self.mArgs.config)
            print("Available configurations are:")
            for c in self.mConfigs:
                print("    " + c)
            print('\n')
            sys.exit(1)

        if self.mArgs.threads != "" and self.mArgs.threads is not None:
            try:
                self.mThreads = int(self.mArgs.threads)
            except:
                print("Provided thread count is invalid, must be an integer")
                sys.exit(1)

            if self.mThreads <= 0:
                print("Provided thread count is invalid, should be 1 or more")
                sys.exit(1)


        if self.mBuildAllPlats or self.mBuildAllConfigs:
            print() # for clear output sake

        self.mBuildDoneFileName = ".build_done"

        print("Build details:")
        if self.mBuildAllPlats:
            print("    Platform: " + str(self.mPlatforms))
        else:
            print("    Platform: " + self.mArgs.plat)
        if self.mBuildAllConfigs:
            print("    Configuration: " + str(self.mConfigs))
        else:
            print("    Configuration: " + self.mArgs.config)
        print("    Clean: " + str(self.mArgs.clean))
        if self.mThreads > 0:
            print("    Thread count: " + str(self.mThreads))
        else:
            print("    Thread count: Max available")

    def AnimProgress(self, stepString, startTime=0):
        self.mAnimDone = False

        for c in itertools.cycle('\\|/-'):
            if self.mAnimDone:
                break
            messageEnd = "..."
            if startTime != 0:
                messageEnd += " (" + SecondsToHumanReadableString(time.perf_counter() - startTime) + ")"
            sys.stdout.write("\r " + c + " ==> " + stepString + messageEnd)
            sys.stdout.flush()
            time.sleep(0.2)

        sys.stdout.write("\r   ==> " + stepString + "... ")
        sys.stdout.flush()

    def InterruptHandler(self, sig, frame):
        self.mAnimDone = True
        if self.mAnimThread is not None and self.mAnimThread.is_alive():
            self.mAnimThread.join()
        sys.stdout.flush()
        print("\nInterrupt captured\n")
        sys.exit(1)

    def CallStage(self, prompt, pargs, stageId=0, stageCount=0, measureTime=False):
        capture = not self.mArgs.verbose

        if (stageCount > 1):
            prompt = str(stageId) + "/" + str(stageCount) + " " + prompt

        if measureTime:
            processStart = time.perf_counter()

        if not self.mArgs.verbose and not self.mArgs.noanim:
            animArgs = [prompt]
            if measureTime:
                animArgs.append(processStart)
            self.mAnimThread = threading.Thread(target=self.AnimProgress,
                                                args=animArgs)
            self.mAnimThread.start()
        else:
            print("   ==> " + prompt + "... ")

        sys.stdout.flush()
        result = subprocess.run(pargs, capture_output=capture)

        if measureTime:
            processTime = time.perf_counter() - processStart

        if self.mAnimThread is not None:
            self.mAnimDone = True
            self.mAnimThread.join()
            self.mAnimThread = None

        if not self.mArgs.verbose:
            if result.returncode == 0:
                resultMsg = "SUCCESS"
            else:
                resultMsg = "FAILED"

            if measureTime:
                resultMsg += " (" + SecondsToHumanReadableString(processTime) + ")"

            if self.mArgs.noanim:
                print("   ==> " + prompt + "... " + resultMsg)
            else:
                print(resultMsg)

        if result.returncode != 0:
            raise Exception("Build failed. Rerun with -v option to see details.")

    def CMakeCreate(self):
        os.chdir(self.mCMakeDir)

        process = [
            "cmake",
            "../../..",
            "-DCMAKE_BUILD_TYPE=" + self.mCurrentConfig,
            "-G", self.mGenerator,
            "-A", self.mCurrentPlat
        ]

        self.CallStage("Creating build files for " + self.mCurrentPlat + "/" + self.mCurrentConfig, process, measureTime=True)

        os.chdir(self.mScriptDir)

    def MSBuild(self, project, target, stageId, stageCount):
        os.chdir(self.mCMakeDir)

        solution = project + ".sln"
        threadArg = "/m"
        if self.mThreads > 0:
            threadArg += ":" + str(self.mThreads)

        process = [
            "msbuild",
            solution,
            threadArg,
            "/t:" + target,
            "/p:Configuration=" + self.mCurrentConfig + ";Platform=" + self.mCurrentPlat
        ]

        if self.mThreads == 1:
            self.CallStage("Building " + target, process, stageId, stageCount, measureTime=True)
        else:
            self.CallStage("Building", process, stageId, stageCount, measureTime=True)

        os.chdir(self.mScriptDir)

    def CheckRequirements(self):
        for r in self.mRequirements:
            if r() is False:
                raise OSError("Build requirements not met")

    def SwitchCWDToScriptRoot(self):
        path = os.path.dirname(os.path.realpath(sys.argv[0]))
        print("   ==> Switching CWD to " + path)
        os.chdir(path)

    def SetupBuildTree(self):
        if not self.mArgs.clean:
            print("   ==> Setting up build tree for " + self.mCurrentPlat + "/" + self.mCurrentConfig + "... ", end='')
        else:
            print("   ==> Cleaning build tree for " + self.mCurrentPlat + "/" + self.mCurrentConfig + "... ", end='')
        self.mCMakeDir = self.mBuildDir / self.mCurrentPlat / self.mCurrentConfig
        self.mOutputDir = self.mBinDir / self.mCurrentPlat / self.mCurrentConfig
        self.mBuildDoneFile = self.mOutputDir / self.mBuildDoneFileName

        if self.mArgs.clean is True:
            try:
                shutil.rmtree(self.mCMakeDir)
                shutil.rmtree(self.mOutputDir)
            except OSError:
                pass

            try:
                os.removedirs(self.mBinDir)
                os.removedirs(self.mBuildDir)
            except OSError:
                pass

            print("DONE")
            return False

        if os.path.isfile(self.mBuildDoneFile):
            print("ALREADY BUILT")
            return False

        try:
            os.makedirs(self.mCMakeDir)
            os.makedirs(self.mOutputDir)
        except OSError:
            pass

        print("DONE")
        return True

    def BuildCurrent(self):
        if self.SetupBuildTree() is False:
            return

        self.CMakeCreate()

        stageId = 0

        if self.mThreads == 1:
            # Build step-by-step on single-threaded build
            for dep in self.mDeps:
                stageId += 1
                self.MSBuild(dep[0], dep[1], stageId, len(self.mDeps))
        else:
            # Multithreaded builds run faster if you trigger just the last step
            # Then MSBuild will be able to work on multiple projects at once
            stageId = 1
            dep = self.mDeps[-1]
            self.MSBuild(dep[0], dep[1], stageId, stageId)

        # touch build done file
        open(self.mBuildDoneFile, 'w').close()

    def Build(self):
        buildStartTime = time.perf_counter()

        print("Build progress:")
        self.SwitchCWDToScriptRoot()
        self.CheckRequirements()
        if self.mBuildAllPlats and self.mBuildAllConfigs:
            for p in self.mPlatforms:
                for c in self.mConfigs:
                    self.mCurrentPlat = p
                    self.mCurrentConfig = c
                    self.BuildCurrent()
        elif self.mBuildAllPlats and not self.mBuildAllConfigs:
            self.mCurrentConfig = self.mArgs.config
            for p in self.mPlatforms:
                self.mCurrentPlat = p
                self.BuildCurrent()
        elif self.mBuildAllConfigs and not self.mBuildAllPlats:
            self.mCurrentPlat = self.mArgs.plat
            for c in self.mConfigs:
                self.mCurrentConfig = c
                self.BuildCurrent()
        else:
            self.mCurrentPlat = self.mArgs.plat
            self.mCurrentConfig = self.mArgs.config
            self.BuildCurrent()

        buildTotalTime = time.perf_counter() - buildStartTime
        print("\nScript is done (took " + SecondsToHumanReadableString(buildTotalTime) + ")\n")


def IsCallable(exe):
    print("   ==> Checking for " + exe + "... ", end='')
    path = shutil.which(exe)
    if path is not None:
        print("OK")
        return True
    else:
        print("NOT FOUND")
        print("Make sure " + exe + " is visible in PATH env variable before using this script.")
        return False

def IsNonEmptyDir(dir):
    if os.path.isdir(dir) and len(os.listdir(dir)) > 0:
        return True
    else:
        return False

def HasCMake():
    return IsCallable("cmake")

def HasMSBuild():
    return IsCallable("msbuild")

def HasSubmodules():
    print("   ==> Checking for submodules... ", end='')
    # TODO move below array to __main__ to have all "configuration" elements
    #      in one common place
    submodules = [
        "cxxopts",
        "freetype2",
        "glslang",
        "googletest",
        "imgui",
        "libpng",
        "libsquish",
        "nfenginedeps",
        "rapidjson",
        "tinyexr",
        "tinyobjloader",
        "tracy",
        "zlib"
    ]

    hasAllSubmodules = True
    for s in submodules:
        if not IsNonEmptyDir(s):
            hasAllSubmodules = False

    if hasAllSubmodules:
        print("OK")
        return True
    else:
        print("FAILED")
        print("Download required submodules via: git submodule update --init --recursive")
        return False

def GetGlslangSPVTools():
    print("   ==> Checking if Glslang has SPIRV-Tools downloaded... ", end='')
    if IsNonEmptyDir("glslang/External/spirv-tools"):
        print("OK")
        return True
    else:
        print("NOT FOUND")
        process = [
            "python",
            "update_glslang_sources.py"
        ]
        print("        \\_ Downloading SPIRV-Tools glslang dependency... ", end='')
        sys.stdout.flush()
        os.chdir("glslang")
        result = subprocess.run(process, capture_output=True)
        os.chdir("..")

        if result.returncode == 0:
            print("OK")
            return True
        else:
            print("FAILED")
            print("Enter glslang directory, run update_glslang_sources.py file and manually check for errors")
            return False


def main():
    reqs = [
        HasCMake,
        HasMSBuild,
        HasSubmodules,
        GetGlslangSPVTools
    ]

    deps = [
        ("nfEngineDeps", "Deps\\zlibstatic"),
        ("nfEngineDeps", "Deps\\png_static"),
        ("nfEngineDeps", "Deps\\squish"),
        ("nfEngineDeps", "Deps\\jpeg"),
        ("nfEngineDeps", "Deps\\glslang\\OSDependent"),
        ("nfEngineDeps", "Deps\\hlsl\\HLSL"),
        ("nfEngineDeps", "Deps\\glslang\\OGLCompiler"),
        ("nfEngineDeps", "Deps\\glslang\\glslang"),
        ("nfEngineDeps", "Deps\\glslang\\SPIRV"),
        ("nfEngineDeps", "Deps\\gtest"),
        ("nfEngineDeps", "Deps\\freetype"),
        ("nfEngineDeps", "Deps\\deps\\miniz"),
        ("nfEngineDeps", "Deps\\tinyexr"),
        ("nfEngineDeps", "Deps\\tinyobjloader"),
        ("nfEngineDeps", "Deps\\D3D12MemoryAllocator"),
        ("nfEngineDeps", "Deps\\NFEDepsPostBuild")
    ]

    plats = [
        "x64"
    ]

    confs = [
        "Release",
        "Debug"
    ]

    try:
        builder = DepsBuilder(projectName="nfEngine",
                              generator="Visual Studio 17 2022",
                              requirements=reqs, deps=deps,
                              platforms=plats, configurations=confs)
        builder.Build()
    except Exception as e:
        print("Exception occured while building:")
        print(e)
        sys.exit(1)


if __name__ == "__main__":
    main()
