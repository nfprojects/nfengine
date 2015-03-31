## @file   xmlParser.py
## @author mkulagowski (mkkulagowski@gmail.com)
## @brief  Module for parsing xml output from gtest


import os.path as OP
import xml.etree.ElementTree as ET
import color
import sys


tabIndent = 4 * ' '


class GtestParser():
    def __init__(self, path):
        self.xmlDoc = ET.parse(OP.normpath(path))
        self.root = {
            "node": self.xmlDoc.getroot(),
            "failNo": 0,
            "testNo": 0}
        self.root['failNo'] = self.root['node'].attrib['failures']
        self.root['testNo'] = self.root['node'].attrib['tests']
        self.colorizerInstance = color.Colorizer()
        self.failLinks = []
        sys.stdout.write('Opened testsuite: ' + self.root['node'].attrib['name'] + '\n')


    def parseXml(self):
        testSuite = self.root['node'].findall('testsuite')
        for suite in testSuite:
            sys.stdout.write(tabIndent + suite.attrib['name'] + ' results:\n')
            testCase = suite.findall('testcase')
            for case in testCase:
                sys.stdout.write(2 * tabIndent)
                if case.find('failure') is None:
                    self.colorizerInstance.printMulti('PASSED ', 'green', None, True)
                else:
                    self.colorizerInstance.printMulti('FAILED ', 'red', None, True)
                    self.failLinks.append(suite.attrib['name'] + '.' + case.attrib['name'])
                self.colorizerInstance.printMulti(case.attrib['name'], 'white', None, True)
                sys.stdout.write('\n')

                
    def parseXmlFailsOnly(self):
        testSuite = self.root['node'].findall('testsuite')
        for suite in testSuite:
            testCase = suite.findall('testcase')
            for case in testCase:
                if case.find('failure') is not None:
                    self.failLinks.append(suite.attrib['name'] + '.' + case.attrib['name'])
        for failLink in self.failLinks:
            dotPosition = failLink.find('.')
            sys.stdout.write(2 * tabIndent)
            self.colorizerInstance.printMulti('FAILED ', 'red', None, True)
            sys.stdout.write(' ' + failLink[:dotPosition] + '\\')
            self.colorizerInstance.printMulti(failLink[dotPosition + 1:], 'white', None, True)
            sys.stdout.write('\n')
            
                
