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


class DepsBuilder:
    def __init__(self, projectName, generator, testDefine, deps, testDeps, platforms, configurations):
        signal.signal(signal.SIGINT, self.InterruptHandler)

        self.mProjectName = projectName
        self.mGenerator = generator
        self.mDeps = deps
        self.mTestDeps = testDeps
        self.mTestDefine = testDefine
        self.mPlatforms = platforms
        self.mConfigs = configurations
        self.mAnimThread = None
        self.mAnimDone = False
        self.mBuildAllPlats = False
        self.mBuildAllConfigs = False

        if platforms is None:
            self.mPlatforms = ["x64"]

        if configurations is None:
            self.mConfigs = ["Release"]

        self.mScriptDir = pathlib.PurePath(os.path.dirname(os.path.realpath(sys.argv[0])))
        self.mBuildDir = self.mScriptDir / 'build'
        self.mBinDir = self.mScriptDir / 'Bin'

        print("=== " + self.mProjectName + " dependency builder ===\n")

        if platform.system() is not "Windows":
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
        parser.add_argument('--tests', dest='tests', action="store_true",
                            help="Include dependencies required for running tests.")

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

        if self.mBuildAllPlats or self.mBuildAllConfigs:
            print() # for clear output sake

        if self.mArgs.tests:
            self.mBuildDoneFileName = ".build_tests_done"
        else:
            self.mBuildDoneFileName = ".build_done"

        if self.mArgs.tests and self.mTestDefine is None:
            Exception("There's no test-related CMake define provided, cannot enable tests")

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
        print("    Build test deps: " + str(self.mArgs.tests) + "\n")

    def AnimProgress(self, stepString):
        self.mAnimDone = False

        for c in itertools.cycle('\\|/-'):
            if self.mAnimDone:
                break
            sys.stdout.write("\r " + c + " ==> " + stepString + "...")
            sys.stdout.flush()
            time.sleep(0.2)

        sys.stdout.write("\r   ==> " + stepString + "... ")
        sys.stdout.flush()

    def IsCallable(self, exe):
        print("   ==> Checking for " + exe + "... ", end='')
        path = shutil.which(exe)
        if path is not None:
            print("FOUND")
            return True
        else:
            print("NOT FOUND")
            print("Make sure " + exe + " is visible in PATH env variable before using this script.")
            return False

    def InterruptHandler(self, sig, frame):
        self.mAnimDone = True
        if self.mAnimThread is not None and self.mAnimThread.is_alive():
            self.mAnimThread.join()
        sys.stdout.flush()
        print("\nInterrupt captured\n")
        sys.exit(1)

    def CallStage(self, prompt, pargs, stageId=0, stageCount=0):
        capture = not self.mArgs.verbose

        if (stageCount > 0):
            prompt = str(stageId) + "/" + str(stageCount) + " " + prompt

        if not self.mArgs.verbose and not self.mArgs.noanim:
            self.mAnimThread = threading.Thread(target=self.AnimProgress,
                                                args=[prompt])
            self.mAnimThread.start()
        else:
            print("   ==> " + prompt + "... ")

        sys.stdout.flush()
        result = subprocess.run(pargs, capture_output=capture)

        if self.mAnimThread is not None:
            self.mAnimDone = True
            self.mAnimThread.join()
            self.mAnimThread = None

        if not self.mArgs.verbose:
            if result.returncode is not 0:
                if self.mArgs.noanim:
                    print("   ==> " + prompt + "... FAILED")
                else:
                    print("FAILED")
                raise Exception("Build failed. Rerun with -v option to see details.")
            else:
                if self.mArgs.noanim:
                    print("   ==> " + prompt + "... SUCCESS")
                else:
                    print("SUCCESS")

    def CMakeCreate(self):
        os.chdir(self.mCMakeDir)

        if self.mArgs.tests:
            enableTests = '1'
        else:
            enableTests = '0'

        process = [
            "cmake",
            "../../..",
            "-DCMAKE_BUILD_TYPE=" + self.mCurrentConfig,
            "-G", self.mGenerator,
            "-A", self.mCurrentPlat
        ]

        if self.mTestDefine is not None:
            process.append("-D" + self.mTestDefine + "=" + enableTests)

        self.CallStage("Creating build files for " + self.mCurrentPlat + "/" + self.mCurrentConfig, process)

        os.chdir(self.mScriptDir)

    def MSBuild(self, project, target, stageId, stageCount):
        os.chdir(self.mCMakeDir)

        solution = project + ".sln"
        process = [
            "msbuild",
            solution,
            "/t:" + target,
            "/p:Configuration=" + self.mCurrentConfig + ";Platform=" + self.mCurrentPlat
        ]
        self.CallStage("Building " + target, process, stageId, stageCount)

        os.chdir(self.mScriptDir)

    def CheckEnv(self):
        if not self.IsCallable("cmake"):
            raise OSError()

        if not self.IsCallable("msbuild"):
            raise OSError()

    def SwitchCWDToScriptRoot(self):
        os.chdir(os.path.dirname(sys.argv[0]))

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

        if self.mArgs.tests:
            stageId = 0
            for testdep in self.mTestDeps:
                stageId += 1
                self.MSBuild(testdep[0], testdep[1], stageId, len(self.mTestDeps))
        else:
            stageId = 0
            for dep in self.mDeps:
                stageId += 1
                self.MSBuild(dep[0], dep[1], stageId, len(self.mDeps))

        # touch build done file
        open(self.mBuildDoneFile, 'w').close()

    def Build(self):
        print("Build progress:")
        self.CheckEnv()
        self.SwitchCWDToScriptRoot()

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

        print("\nScript is done\n")


def main():
    deps = [
        ("nfEngineDeps", "zlibstatic"),
        ("nfEngineDeps", "png_static"),
        ("nfEngineDeps", "squish"),
        ("nfEngineDeps", "jpeg"),
        ("nfEngineDeps", "glslang\OSDependent"),
        ("nfEngineDeps", "hlsl\HLSL"),
        ("nfEngineDeps", "glslang\OGLCompiler"),
        ("nfEngineDeps", "glslang\glslang"),
        ("nfEngineDeps", "glslang\SPIRV"),
        ("nfEngineDeps", "gtest"),
        ("nfEngineDeps", "freetype"),
        ("nfEngineDeps", "NFEDepsPostBuild")
    ]

    testDeps = [
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
                              generator="Visual Studio 16 2019",
                              testDefine="",
                              deps=deps, testDeps=testDeps,
                              platforms=plats, configurations=confs)
        builder.Build()
    except Exception as e:
        print("Exception occured while building:")
        print(e)
        sys.exit(1)


if __name__ == "__main__":
    main()
