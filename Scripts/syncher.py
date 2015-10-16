#!/usr/bin/env python

## @file   syncher.py
## @author mkulagowski (mkkulagowski@gmail.com)
## @brief  Multiplatform ftp synchronization script


import argparse, os, time, sys
from ftplib import FTP
from shutil import rmtree

dataDir = '/nfEngineDemo'
dataSubDir = './Data'
dataDirectory = '/nfEngineDemo/Data'
ftp = FTP("nfprojects.org")


def GetDirTreeRemote(dirName, dirList):
    ftp.cwd(dirName)
    lst = []
    ftp.retrlines('LIST',lst.append)
    for line in lst:
        name = line[line.rfind(' ')+1:]
        if line[0] is 'd':
            dirList.append(dirName + '/' + name)
            ftp.cwd(dataDirectory)
            GetDirTreeRemote(dirName + '/' + name, dirList)
            ftp.cwd('..')
    return


def GetFileListRemote(dirName, fileList):
    ftp.cwd(dirName)

    ### FOR USE IN THE NEXT COMMIT
    #tlst = []
    #a.retrlines('MLST',tlst.append)
    #for tline in tlst:
    #    tline = tline[6:20]
    ###

    lst = []
    ftp.retrlines('LIST',lst.append)
    for line in lst:
        name = line[line.rfind(' ')+1:]
        if line[0] is 'd':
            ftp.cwd(dataDirectory)
            GetFileList(dirName + '/' + name, fileList, False)
            ftp.cwd('..')
        else:
            fileList.append(dirName + '/' + name)
    return


def GetFileListLocal(dirName, fileList):
    for path, dirs, files in os.walk(dirName):
        for file in files:
            fileList.append(os.path.join(path, file))
    return


def GetFileList(dirName, fileList, isLocal):
    if isLocal is True:
        GetFileListLocal(dirName, fileList)
    else:
        GetFileListRemote(dirName, fileList)
    return


def main(argv):
    # managing input arguments
    argParser = argparse.ArgumentParser(description='nfProjects FTP server synchronization tool.')
    argParser.add_argument('Mode', choices=['i'],# 'p', 'u'],
                            help="'i' is used to initialize the Data folder;")
                                #+"'p' pushes current data from the directory to the server; "
                                #+"'u' downloads data from server to repo, if needed")
    args = argParser.parse_args()

    # playing with paths
    dataPath = os.path.realpath(__file__)
    dataPath = os.path.join(dataPath, os.path.pardir, os.path.pardir)
    dataPath = os.path.normpath(dataPath + dataDir)

    ftp.login("guest")
    os.chdir(dataPath)
    isDataExist = os.path.isdir(dataSubDir)

    # if Data folder exists, delete it after prompting
    if isDataExist is True:
        toDelete = ''
        while toDelete not in ['y', 'n']:
            toDelete = raw_input('Data folder exists. Do You want to delete contents and download files from server? (Y/N) ')
            toDelete = toDelete.lower()[0]

        if toDelete is 'n':
            exit()
        else:
            rmtree(dataSubDir)

    os.mkdir(dataSubDir)
    os.chdir(dataSubDir)

    ftp.cwd(dataDirectory)
    remoteFileList = []

    # Get directory structure and recreate it locally
    GetDirTreeRemote('.', remoteFileList)

    for file in remoteFileList:
        if os.path.isdir(file) is False:
            os.mkdir(file)

    remoteFileList = []

    # Get file list from the server
    GetFileList('.', remoteFileList, False)

    # Recreate each file from the server locally
    for file in remoteFileList:
        with open(file, 'wb') as ftpfile:
            resp = ftp.retrbinary('RETR ' + file, ftpfile.write)
            print 'DOWNLOADING', file
    print ('##DONE##')


if __name__ == '__main__':
    main(sys.argv)