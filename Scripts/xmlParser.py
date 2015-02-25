## @file   xmlParser.py
## @author mkulagowski (mkkulagowski@gmail.com)
## @brief  Module for parsing xml output from gtest


import os.path as OP
import xml.etree.ElementTree as ET
import color


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
        print 'Opened testsuite: ' + self.root['node'].attrib['name']


    def parseXml(self):
        testSuite = self.root['node'].findall('testsuite')
        for suite in testSuite:
            print tabIndent + suite.attrib['name'] + ' results:'
            testCase = suite.findall('testcase')
            for case in testCase:
                if case.find('failure') is None:
                    self.colorizerInstance.printMulti(2 * tabIndent + 'PASSED ', 'green', None, True)
                else:
                    self.colorizerInstance.printMulti(2 * tabIndent + 'FAILED ', 'red', None, True)
                    self.failLinks.append(suite.attrib['name'] + '.' + case.attrib['name'])
                self.colorizerInstance.printMulti(case.attrib['name'] + '\n', 'white', None, True)
