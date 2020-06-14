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
        print('Opened testsuite: ' + self.root['node'].attrib['name'])


    def parseXml(self):
        testSuite = self.root['node'].findall('testsuite')
        for suite in testSuite:
            print("{}{} results:".format(tabIndent,suite.attrib['name']))
            testCase = suite.findall('testcase')
            for case in testCase:
                print(2 * tabIndent, end='')
                if case.find('failure') is None:
                    self.colorizerInstance.printMulti('PASSED ', 'green', None, True)
                else:
                    self.colorizerInstance.printMulti('FAILED ', 'red', None, True)
                    self.failLinks.append(suite.attrib['name'] + '.' + case.attrib['name'])
                self.colorizerInstance.printMulti(case.attrib['name'], 'white', None, True)
                print('')


    def parseXmlFailsOnly(self):
        testSuite = self.root['node'].findall('testsuite')
        for suite in testSuite:
            testCase = suite.findall('testcase')
            for case in testCase:
                if case.find('failure') is not None:
                    self.failLinks.append(suite.attrib['name'] + '.' + case.attrib['name'])
        for failLink in self.failLinks:
            dotPosition = failLink.find('.')
            print(2 * tabIndent, end='')
            self.colorizerInstance.printMulti('FAILED ', 'red', None, True)
            print(' ' + failLink[:dotPosition] + '\\', end='')
            self.colorizerInstance.printMulti(failLink[dotPosition + 1:], 'white', None, True)
            print('')
        if len(self.failLinks) <= 0:
            self.colorizerInstance.printMulti(2 * tabIndent + 'ALL TESTS PASSED\n', 'green', None, True)


