#!/usr/bin/env python

## @file   tests.py
## @author mkulagowski (mkkulagowski@gmail.com)
## @brief  Multiplatform testing script


import subprocess, shlex, time, xmlParser, os, sys, argparse


def gatherFailed(path, name, failLinks):
    fails = []
    failString = '-n {} -p {} --args --gtest_filter='.format(name, path)
    for fail in failLinks:
        failString += (fail + ':')
    fails.append(failString[:-1] + '')
    return fails


def nameFromPath(path):
    normpath = os.path.normpath(path)
    if sys.platform == "win32":
        return normpath[normpath.rfind('\\')+1:normpath.rfind('.exe')]
    return normpath[normpath.rfind('/')+1:]


def typeFromPath(path):
    normpath = os.path.normpath(path)
    if sys.platform == "win32":
        return normpath[normpath.rfind('in\\')+len('in\\'):normpath.rfind('\\')]
    return normpath[normpath.rfind('in/')+len('in/'):normpath.rfind('/')]


def findTestsIntoPaths(path, testlist, isPath):
    pathlist = []

    if isPath:
        for root1, dirs1, files1 in os.walk(path):
            for file1 in files1:
                if str(file1).lower() in testlist:
                    pathlist.append(os.path.normpath(root1 + '/' + file1))
    else:
        for root1, dirs1, files1 in os.walk(path):
            for dir1 in dirs1:
                for root2, dirs2, files2 in os.walk(root1 + '/' + dir1):
                    for dir2 in dirs2:
                        if str(dir2).lower() in ['release', 'debug']:
                            for root3, dirs3, files3 in os.walk(root2 + '/' + dir2):
                                for file3 in files3:
                                    if sys.platform == "win32":
                                        # Don't ask me, Windows has its needs...
                                        f = str(file3).lower()
                                    else:
                                        f = str(file3)
                                    if f in testlist:
                                        pathlist.append(os.path.normpath(root3 + '/' + file3))
    return pathlist


def runTest(args, isVerbose):
    animation = "|/-\\"
    testProcess = subprocess.Popen(args, stdout = subprocess.PIPE)
    linesIterator = iter(testProcess.stdout.readline, b"")
    animationIter = 0
    for line in linesIterator:
        if isVerbose is True:
            print(line),
        else:
            print animation[animationIter % len(animation)] + "\r",
            animationIter += 1
            time.sleep(0.1)


def main(argv):
    # initializing needed variables
    fails = []

    testList = ['nfCommonTest', 'nfCoreTest', 'nfRendererTest']

    # managing input arguments
    argParser = argparse.ArgumentParser(description='Test script for nfengine project.')
    argParser.add_argument('-v', '--verbose', action='store_true',
                            help='Instead of parsing output xml, script pipes gtests output')
    argParser.add_argument('-p', '--path', metavar='testPath', help=('Runs gtest files from given'
                           'path. Found tests will be listed for choosing.'))
    argParser.add_argument('-n', '--name', metavar='testName',
                            help='Runs gtest files with given name')
    argParser.add_argument('-q', '--quiet', action='store_true', help='Suppresses output')
    argParser.add_argument('-f', '--perf', action='store_true', help='Adds Performance tests to test list')
    argParser.add_argument('--args', default=[], metavar='gtestArguments', nargs=argparse.REMAINDER,
                            help='Pipes args to gtest. For available options see gtest manual.')
    args = argParser.parse_args()

    if args.perf:
        testList.append('nfCommonPerfTest')

    # playing with paths
    currentPath = os.path.realpath(__file__)
    xmlPath = os.path.normpath(os.getcwd() + '/test_detail.xml')
    currentPath = os.path.join(currentPath, os.path.pardir, os.path.pardir)
    currentPath = os.path.normpath(currentPath)

    # parsing -n and -p arguments
    findTests = True
    findPath = currentPath + '/Bin'
    if args.path is not None:
        findPath += '/' + args.path
    if args.name is not None:
        testList = [args.name.lower()]
    if args.path is not None and args.name is not None:
        findTests = False
        pathList = [findPath + '/' + args.name]

    # searching for tests if needed
    findPath = os.path.normpath(findPath)
    if findTests is True:
        print 'Current directory is ' + currentPath
        print '\nLooking for test files in {}...'.format(findPath)

        # adding .exe on win if user forgot
        if sys.platform == "win32":
            testList = [test.lower() if test.endswith('.exe') else test.lower() + '.exe'
                        for test in testList]
        # running search to fill pathList
        isPath = args.path is not None
        pathList = findTestsIntoPaths(findPath, testList, isPath)
        print '{} test files found.\n'.format(len(pathList))

    # parsing gtest arguments
    gtestArgs = ''
    if len(args.args) > 0:
        gtestArgs = args.args[0]

    # running the loop
    if len(pathList) > 0:
        print '\nRunning found tests ====='

    for path in pathList:
        testPath = os.path.normpath(path)

        # check for '.exe' if ran under windows os
        if sys.platform == "win32" and not testPath.endswith('.exe'):
            testPath += '.exe'

        # check if test file is valid
        if os.path.isfile(testPath) and os.access(testPath, os.X_OK):
            print '\n====Running {} for {}'.format(nameFromPath(testPath), typeFromPath(testPath))

            # building args for subprocess
            testArgs = [testPath, '--gtest_output=xml', gtestArgs]

            # running subprocess
            runTest(testArgs, args.verbose)

            # managing test's output xml
            print '====Parsing {} XML'.format(nameFromPath(testPath))
            if os.path.exists(xmlPath):
                testParser = xmlParser.GtestParser(xmlPath)
                if args.quiet:
                    testParser.parseXmlFailsOnly()
                else:
                    testParser.parseXml()

                # gathering failed tests
                if len(testParser.failLinks) > 0:
                    fails.append(gatherFailed(typeFromPath(path), nameFromPath(path),
                                                    testParser.failLinks))
                os.remove(xmlPath)
            else:
                print '====Output XML for {} not found in: {}'.format(nameFromPath(testPath),xmlPath)

        else:
            print '===={} NOT FOUND!'.format(nameFromPath(testPath).upper())

    if len(fails) > 0:
        print ('\n====\nTo run again failed tests, use commands shown below:\n')

        for fail in fails:
            print 'python ' + os.path.normpath(os.path.realpath(__file__)),
            print ' ' + fail[0]


if __name__ == '__main__':
    if sys.platform == 'linux' or sys.platform == 'linux2' or sys.platform == 'win32':
        main(sys.argv)
    else:
        print 'ERROR: Platform not supported!'
        exit(1)
