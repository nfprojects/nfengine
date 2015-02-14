import subprocess, shlex, time, xmlParser, os, sys, getopt

def main(argv):
  #initializing needed variables
  SUPPRESS = False
  FILTERS = []
  FAILS = []
  LIST_TESTS = False

  pathList = [
    ('Win32/Debug', 'Bin/Win32/Debug/'),
    ('Win32/Release', 'Bin/Win32/Release/'),
    ('x64/Debug', 'Bin/x64/Debug/'),
    ('x64/Release', 'Bin/x64/Release/')]

  testList = ['nfCommon']#, 'nfCore']

  animation = "|/-\\"
  #managing input arguments
  try:
    opts, args = getopt.getopt(argv,'hsf:l',['filter=','help', 'suppress', 'list_tests'])
  except getopt.GetoptError:
    print 'Acceptable arguments: -h (--help), -s (--suppress), -f <gtest_filters> (--filter <gtest_filters>), -l (--list_tests)'
    sys.exit(2)
  for opt, arg in opts:
    if opt in ("-h", "--help"):
      print ('Acceptable arguments:\n\t-h (--help)\n'
            '\t\t\tto display this text\n'
            '\t-s (--suppress)\n'
            '\t\t\tto suppress test output and print only results\n'
            '\t-f <gtest_filters> (--filter <gtest_filters>)\n'
            '\t\t\tto run only selected tests, filter format: \'t_n;tv_n;f_t\'\n\t\t\t\tt_n - test number, can be found below\n'
            '\t\t\t\ttv_n - test version number, can be found below\n\t\t\t\tf_t - filter text, for filtering see gtest documentation\n'
            '\t-l (--list_tests)\n'
            '\t\t\t\tto list testcases from found tests (usable for filtering)')
      print '\n===== Tests ====='
      no = 0
      for test in testList:
        print str(no) + ' - ' + test
        no += 1
      print '\n===== Test versions ====='
      no = 0
      for path in pathList:
        print str(no) + ' - ' + path[0]
        no += 1
      sys.exit(0)
    elif opt in ("-s", "--suppress"):
      SUPPRESS = True
    elif opt in ("-f", "--filter"):
      FILTERS.append(opt)
    elif opt in ("-l", "--list_tests"):
      LIST_TESTS = True
      
  #playing with paths
  #--
  currentPath = os.path.realpath(__file__)
  xmlPath = os.path.join(currentPath, os.path.pardir)
  xmlPath = os.path.normpath(xmlPath + '/test_detail.xml')
  currentPath = os.path.join(currentPath, os.path.pardir, os.path.pardir)
  currentPath = os.path.normpath(currentPath)
  print 'Current directory is ' + currentPath

  #running test loop
  #--
  length = len(FILTERS)
  if len(FILTERS) == 0:
    length = 1
  for FILTER in range(0, length):
    #swapping tables if filters were used
    if len(FILTERS) > 0:
      _pathList = pathList
      _testList = testList
      _filter=[None, None, None]
      for i in range(0,2):
        semicolon = FILTER.find(';')
        _filter[i] = FILTER[:semicolon]
        FILTER = FILTER[semicolon + 1:]
      _filter[2] = FILTER
      pathList = _pathList[_filter[0]]
      testList = _testList[_filter[1]]
    #running the loop
    for test in testList:
      print '\n'
      print '===== Running ' + test + ' tests ====='
      for path in pathList:
        print '\n'
        print 'Checking if tests are compiled for ' + path[0] + '...'
        testPath = os.path.join(currentPath, os.path.normpath(path[1] + test + 'Test.exe'))
        if os.path.exists(testPath):
          print 'FOUND'
          print 'Running ' + test + ' tests in ' + path[0]
          #building args for subprocess
          args = [testPath, '--gtest_output=xml']
          if LIST_TESTS is True:
            args.append('--gtest_list_tests')
          #running subprocess
          testProcess = subprocess.Popen(args, stdout = subprocess.PIPE)
          #subprocess output
          lines_iterator = iter(testProcess.stdout.readline, b"")
          animationIter = 0
          for line in lines_iterator:
            if SUPPRESS is False:
              print(line)
            else:
              print animation[animationIter % len(animation)] + "\r",
              animationIter += 1
              time.sleep(0.1)
          #managing test's output xml
          if os.path.exists(xmlPath):
            if SUPPRESS is True:
              testParser = xmlParser.gtestParser(xmlPath)
              testParser.parseTheShitOuttaThisBitch()
              #gathering failed tests
              if len(testParser.fail_links) > 0:
                if len(FILTERS) > 0:
                  path_pos = [i for i, t in enumerate(_pathList) if t[0] == path]
                  test_pos = [i for i, t in enumerate(_testList) if t[0] == test]
                else:
                  path_pos = [i for i, t in enumerate(pathList) if t[0] == path]
                  test_pos = [i for i, t in enumerate(testList) if t[0] == test]
                failString = '-f \'' + test_pos + ';' + path_pos + ';'
                for fail in testParser.fail_links:
                  failString += (fail + ':')
                FAILS.append(failString[:-1] + '\'')
            os.remove(xmlPath)
        else:
          print 'NOT FOUND'
  if len(FAILS) > 0:
    print ('\n ===== FAILS =====\n'
          'To run again failed tests, use command shown below:\n'
          'python ' + os.path.normpath(os.path.realpath(__file__)) + ' -s'),
    for fail in FAILS:
      print ' ' + fail,
    print '\n =====       =====\n'
    
if __name__ == "__main__":
  main(sys.argv[1:])
