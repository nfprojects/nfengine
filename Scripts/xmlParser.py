import os.path as OP
import xml.etree.ElementTree as ET
import color


class gtestParser():
    def __init__(self, path):
        self.xmldoc = ET.parse(OP.normpath(path))
        self.root = {
            "node" : self.xmldoc.getroot(),
            "failNo" : 0,
            "testNo" : 0}
        self.root['failNo'] = self.root['node'].attrib['failures']
        self.root['testNo'] = self.root['node'].attrib['tests']
        self.colorizerInstance = color.colorizer()
        self.fail_links = []
        print 'Opened testsuite: ' + self.root['node'].attrib['name']

    def parseTheShitOuttaThisBitch(self):
        testsuite = self.root['node'].findall('testsuite')
        for suite in testsuite:
            print '\t' + suite.attrib['name'] + ' results:'
            testcase = suite.findall('testcase')
            for case in testcase:
                if case.find('failure') is None:
                    self.colorizerInstance.printMulti('\t\tPASSED ', 'green', None, True)
                else:
                    self.colorizerInstance.printMulti('\t\tFAILED ', 'red', None, True)
                    self.fail_links.append(case.attrib['name'])

                self.colorizerInstance.printMulti(case.attrib['name'] + '\n', 'white', None, True)
