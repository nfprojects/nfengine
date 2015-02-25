#!/usr/bin/env python

## @file   tests.py
## @author mkulagowski (mkkulagowski@gmail.com)
## @brief  Multiplatform testing script


import subprocess, shlex, time, xmlParser, os, sys, argparse


def gatherFailed(paths, tests, path, test, failLinks):
    fails = []
    pathPos = [i for i, t in enumerate(paths) if t == path]
    testPos = [i for i, t in enumerate(tests) if t == test]
    failString = '-s {};{} --args --gtest_filters='.format(testPos[0], pathPos[0])
    for fail in failLinks:
        failString += (fail + ':')
    fails.append(failString[:-1] + '')
    return fails


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
    suppress = False
    filters = []
    fails = []
    listTests = False

    pathList = [
        ('Win32/Debug', 'Bin/Win32/Debug/'),
        ('Win32/Release', 'Bin/Win32/Release/'),
        ('x64/Debug', 'Bin/x64/Debug/'),
        ('x64/Release', 'Bin/x64/Release/')]

    testList = ['nfCommon']#, 'nfCore']

    # creating epilogue text
    parserEpilog = '\n\n===== testNo ====='
    no = 0
    for test in testList:
        parserEpilog += '\n' + str(no) + ' - ' + test
        no += 1
    parserEpilog += '\n\n===== verNo ====='
    no = 0
    for path in pathList:
        parserEpilog += '\n' + str(no) + ' - ' + path[0]
        no += 1

    # managing input arguments
    argParser = argparse.ArgumentParser(description='Test script for nfengine project.',
                                        epilog=parserEpilog,
                                        formatter_class=argparse.RawDescriptionHelpFormatter)
    argParser.add_argument('-v', '--verbose', action='store_true',
                            help='Instead of parsing output xml, script pipes gtests output')
    argParser.add_argument('-s', '--single', default='', metavar='tNo;vNo',
                                help='Runs single test. testNo and verNo are listed at the bottom.')
    argParser.add_argument('--args', default=[], metavar='gtestArguments', nargs=argparse.REMAINDER,
                            help='Pipes args to gtest. For available options see gtest manual.')
    args = argParser.parse_args()

    # playing with paths
    currentPath = os.path.realpath(__file__)
    xmlPath = os.path.normpath(os.getcwd() + '/test_detail.xml')
    currentPath = os.path.join(currentPath, os.path.pardir, os.path.pardir)
    currentPath = os.path.normpath(currentPath)
    print 'Current directory is ' + currentPath

    # swapping tables if filters were used
    if len(args.single) > 0:
        _pathList = pathList
        _testList = testList
        _filter = [None, None]
        singleArg = args.single
        semicolon = singleArg.find(';')
        _filter[0] = int(singleArg[:semicolon])
        _filter[1] = int(singleArg[semicolon + 1:])
        if len(pathList) > _filter[1]:
            pathList = [_pathList[_filter[1]]]
        else:
            pathList = []
        if len(testList) > _filter[0]:
            testList = [_testList[_filter[0]]]
        else:
            testList = []

    # parsing gtest arguments
    gtestArgs = ''
    if len(args.args) > 0:
        gtestArgs = args.args[0]

    # running the loop
    for test in testList:
        print '===== Running ' + test + ' tests ====='

        for path in pathList:
            print 'Checking if tests are compiled for {}...'.format(path[0])
            testPath = os.path.join(currentPath, os.path.normpath(path[1] + test + 'Test'))
            # append '.exe' if ran under windows os
            if sys.platform == "win32":
                testPath += '.exe'
                
            if os.path.exists(testPath):
                print 'FOUND'
                print 'Running {} tests in {}'.format(test, path[0])

                # building args for subprocess
                testArgs = [testPath, '--gtest_output=xml', gtestArgs]

                # running subprocess
                runTest(testArgs, args.verbose)

                # managing test's output xml
                print '===== PARSING XML ====='
                if os.path.exists(xmlPath):
                    testParser = xmlParser.GtestParser(xmlPath)
                    testParser.parseXml()

                    # gathering failed tests
                    if len(testParser.failLinks) > 0:
                        if len(args.single) > 0:
                            fails.append(gatherFailed(_pathList, _testList,
                                                      path, test, testParser.failLinks))
                        else:
                            fails.append(gatherFailed(pathList, testList,
                                                      path, test, testParser.failLinks))
                    os.remove(xmlPath)
                else:
                    print 'Output xml not found in: ' + xmlPath

            else:
                print 'NOT FOUND'

    if len(fails) > 0:
        print ('\n ===== FAILS =====\n'
                'To run again failed tests, use command shown below:\n')

        for fail in fails:
            print 'python ' + os.path.normpath(os.path.realpath(__file__)),
            print ' ' + fail[0]
        print '\n =====       =====\n'


if __name__ == '__main__':
    if sys.platform == 'linux' or sys.platform == 'linux2' or sys.platform == 'win32':
        main(sys.argv)
    else:
        print 'ERROR: Platform not supported!'
        exit(1)
