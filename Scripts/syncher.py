#!/usr/bin/env python

## @file
## @author mkulagowski (mk.kulagowski(*AT*)gmail.com)
## @brief  Multiplatform ftp synchronization script


import argparse, os, time, sys, posixpath, getpass
from datetime import datetime
from ftplib import FTP
from shutil import rmtree

DATA_DIR = '/nfEngineDemo/'
DATA_SUB_DIR = './Data/'
DATA_DIRECTORY = DATA_DIR + DATA_SUB_DIR
ftp = FTP("nfprojects.org")

lr_diff = lambda l, r: list(set(l).difference(r))
lr_intr = lambda l, r: list(set(l).intersection(r))
unixPath = lambda path: posixpath.join(*path.split('\\'))


def GetDirTreeRemote(dirName, dirList):
    ftp.cwd(unixPath(dirName))
    list = []
    ftp.retrlines('LIST', list.append)
    for line in list:
        name = line[line.rfind(' ') + 1:]
        if line[0] is 'd':
            dirPath = os.path.normpath((os.path.join(dirName, name)))
            dirList.append(dirPath)
            ftp.cwd(DATA_DIRECTORY)
            GetDirTreeRemote(dirPath, dirList)
            ftp.cwd('..')
    return


def GetDirTreeLocal(dirName, dirList):
    for path, dirs, files in os.walk(dirName):
        for dir in dirs:
            dirList.append(os.path.normpath(os.path.join(path, dir)))
    return


def GetDirTree(dirName, dirList, isLocal):
    if isLocal is True:
        GetDirTreeLocal(dirName, dirList)
    else:
        GetDirTreeRemote(dirName, dirList)
    return


def GetFileListRemote(dirName, fileList):
    ftp.cwd(unixPath(dirName))
    list = []
    ftp.retrlines('MLSD', list.append)
    datePos = len('modify=')
    for line in list:
        fileLine = []
        fileLine.append(os.path.normpath(os.path.join(dirName, line[line.rfind(' ') + 1:])))
        fileLine.append(line[datePos : datePos + 14])
        typePos = line.find('type=') + len('type=')
        fileType = line[typePos : typePos + 3]
        
        if fileType == 'dir':
            ftp.cwd(DATA_DIRECTORY)
            GetFileListRemote(fileLine[0], fileList)
            ftp.cwd('..')
        elif fileType == 'fil':
            fileList.append(fileLine)
    return


def GetFileListLocal(dirName, fileList):
    for path, dirs, files in os.walk(dirName):
        for file in files:
            fileList.append(os.path.normpath(os.path.join(path, file)))
    return


def GetFileList(dirName, fileList, isLocal):
    if isLocal is True:
        GetFileListLocal(dirName, fileList)
    else:
        GetFileListRemote(dirName, fileList)
    return


def listChanges(array, thing, action):
    if array:
        print "\n#{} to {}:".format(thing, action)
        for file in array:
            print os.path.normpath(file)
            

def FileComparisionLists(dirsAdded, dirsDeleted, filesAdded, filesDeleted, filesModifiedLocal, filesModifiedRemote):
    # enter Data directory
    ftp.cwd(DATA_DIRECTORY)

    # get directory trees
    dirsLocal = []
    dirsRemote = []
    GetDirTree('.', dirsLocal, True)
    GetDirTree('.', dirsRemote, False)

    # calculate differences between directory trees
    for dir in lr_diff(dirsLocal, dirsRemote):
        dirsAdded.append(dir)

    for dir in lr_diff(dirsRemote, dirsLocal):
        dirsDeleted.append(dir)

    # get files lists
    filesLocal = []
    filesRemoteMap = []
    GetFileList('.', filesLocal, True)
    GetFileList('.', filesRemoteMap, False)
    
    # sort remote files list
    filesRemoteMap.sort(key=lambda x: x[0].lower())
    
    filesRemoteList = []
    for file in filesRemoteMap:
        filesRemoteList.append(os.path.normpath(file[0]))


    # calculate differences between lists and duplicates
    for file in lr_diff(filesLocal, filesRemoteList):
        filesAdded.append(file)

    for file in lr_diff(filesRemoteList, filesLocal):
        filesDeleted.append(file)

    duplicatesList = list(lr_intr(filesLocal, filesRemoteList))

    # check modification time of duplicated files
    for file in duplicatesList:
        dateLocal = datetime.fromtimestamp(os.path.getmtime(file))
        dateRemote = next(x[1] for x in filesRemoteMap if file == x[0])
        dateRemote = datetime.strptime(dateRemote, "%Y%m%d%H%M%S")

        if dateLocal > dateRemote:
            filesModifiedLocal.append(file)
        elif dateRemote > dateLocal:
            filesModifiedRemote.append(file)   
    return


def InitMode():
    isDataExist = os.path.isdir(DATA_SUB_DIR)
    # if Data folder exists, delete it after prompting
    if isDataExist is True:
        toDelete = ''
        while toDelete not in ['y', 'n']:
            toDelete = raw_input('Data folder exists. Do You want to delete contents and download '
                                + 'files from server? (Y/N) ')
            toDelete = toDelete.lower()[0]

        if toDelete is 'n':
            exit()
        else:
            rmtree(DATA_SUB_DIR)

    os.mkdir(DATA_SUB_DIR)
    os.chdir(DATA_SUB_DIR)

    ftp.cwd(DATA_DIRECTORY)
    remoteFileList = []

    # Get directory structure and recreate it locally
    GetDirTree('.', remoteFileList, False)

    for file in remoteFileList:
        if os.path.isdir(file) is False:
            os.mkdir(file)

    remoteFileList = []

    # Get file list from the server
    GetFileList('.', remoteFileList, False)

    remoteFileList.sort(key=lambda x: x[0].lower())

    # Recreate each file from the server locally
    fileCounter = 1
    totalFilesNo = len(remoteFileList)
    for file in remoteFileList:
        print 'DOWNLOADING {0}/{1}: {2}'.format(fileCounter, totalFilesNo, file[0])
        with open(file[0], 'wb') as ftpfile:
            resp = ftp.retrbinary('RETR ' + unixPath(file[0]), ftpfile.write)
        fileCounter += 1
        fileDate = time.mktime(datetime.strptime(file[1], "%Y%m%d%H%M%S").timetuple())
        os.utime(file[0], (fileDate, fileDate))
            
    return


def PushMode():
    os.chdir(DATA_SUB_DIR)
    dirsAdded = []
    dirsDeleted = []
    filesAdded = []
    filesDeleted = []
    filesModifiedLocal = []
    filesModifiedRemote = []

    # get files and dirs list
    FileComparisionLists(dirsAdded, dirsDeleted, filesAdded, filesDeleted, filesModifiedLocal, filesModifiedRemote)

    # if there are any uncommited changes, abort
    if filesModifiedRemote:
       print "Changes in these files has not been updated from the server:"
       for file in filesModifiedRemote:
           print os.path.normpath(file)
       print "Please update Your files before pushing."
       exit()

    # list changes for user
    listChanges(dirsDeleted, "Directiories", "delete")
    listChanges(dirsAdded, "Directiories", "add")
    listChanges(filesDeleted, "Files", "delete")
    listChanges(filesAdded, "Files", "add")
    listChanges(filesModifiedLocal, "Files", "update")

    # count number of the operations to do
    operationCounter = 1
    totalOperations = len(dirsDeleted) + len(dirsAdded) + len(filesDeleted) + len(filesAdded) + len(filesModifiedLocal)

    if totalOperations == 0:
        print "Nothing to push."
        exit()

    # ask for a permission to proceed
    toPush = ''
    while toPush not in ['y', 'n']:
        toPush = raw_input('\nDo You want to commit these changes? (Y/N) ')
        toPush = toPush.lower()[0]

    if toPush is 'n':
        exit()
    
    # log in as some user, instead of a guest
    print "Push operation requires FTP account with proper rights."    
    login = raw_input("FTP login: ")
    passwd = getpass.getpass("FTP passwd: ")
    ftpAuth = FTP("nfprojects.org")
    ftpAuth.login(login, passwd)
    ftpAuth.cwd(DATA_DIRECTORY)
    
    #begin push process
    if dirsAdded:
        for dir in dirsAdded:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            ftpAuth.mkd(unixPath(dir))
            operationCounter += 1
    
    if filesAdded:
       for file in filesAdded:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            with open(file, 'rb') as ftpfile:
                ftpAuth.storbinary('STOR ' + unixPath(file), ftpfile)
            fileDate = ftp.sendcmd('MDTM ' + unixPath(file))[4:]
            fileDate = time.mktime(datetime.strptime(fileDate, "%Y%m%d%H%M%S").timetuple())
            os.utime(file, (fileDate, fileDate))
            operationCounter += 1

    if filesDeleted:
        for file in filesDeleted:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            ftpAuth.delete(unixPath(file))
            operationCounter += 1

    if dirsDeleted:
        for dir in dirsDeleted:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            ftpAuth.rmd(unixPath(dir))
            operationCounter += 1

    if filesModifiedLocal:
        for file in filesModifiedLocal:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            ftpAuth.delete(unixPath(file))
            with open(file, 'rb') as ftpfile:
               ftpAuth.storbinary('STOR ' + unixPath(file), ftpfile)
            fileDate = ftp.sendcmd('MDTM ' + unixPath(file))[4:]
            fileDate = time.mktime(datetime.strptime(fileDate, "%Y%m%d%H%M%S").timetuple())
            os.utime(file, (fileDate, fileDate))
            operationCounter += 1
    
    return


def UpdateMode():
    os.chdir(DATA_SUB_DIR)
    dirsToAdd = []
    dirsToDelete = []
    filesToAdd = []
    filesToDelete = []
    filesToModifyLocal = []
    filesToModifyRemote = []

    # get files and dirs list
    FileComparisionLists(dirsToDelete, dirsToAdd, filesToDelete, filesToAdd, filesToModifyRemote, filesToModifyLocal)

    # if there are any uncommited changes, abort
    if filesToModifyRemote:
       print "Changes in these files weren't pushed to the server:"
       for file in filesToModifyRemote:
           print os.path.normpath(file)
       print "Please get rid if them, before updating."
       exit()

    # list changes for user
    listChanges(dirsToDelete, "Directiories", "delete")
    listChanges(dirsToAdd, "Directiories", "add")
    listChanges(filesToDelete, "Files", "delete")
    listChanges(filesToAdd, "Files", "add")
    listChanges(filesToModifyLocal, "Files", "update")

    # count number of the operations to do
    operationCounter = 1
    totalOperations = len(dirsToDelete) + len(dirsToAdd) + len(filesToDelete) + len(filesToAdd) + len(filesToModifyLocal)

    if totalOperations == 0:
        print "Nothing to update."
        exit()

    # ask for a permission to proceed
    toUpdate = ''
    while toUpdate not in ['y', 'n']:
        toUpdate = raw_input('\nDo You want to commit these changes? (Y/N) ')
        toUpdate = toUpdate.lower()[0]

    if toUpdate is 'n':
        exit()

    # get remote files map for files dates
    filesRemoteMap = []
    GetFileList('.', filesRemoteMap, False)

    # begin update process
    if dirsToAdd:
        for dir in dirsToAdd:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            os.mkdir(dir)
            operationCounter += 1
    
    if filesToAdd:
       for file in filesToAdd:
           print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
           with open(file, 'wb') as ftpfile:
               ftp.retrbinary('RETR ' + unixPath(file), ftpfile.write)    
           fileDate = ftp.sendcmd('MDTM ' + unixPath(file))[4:]
           fileDate = time.mktime(datetime.strptime(fileDate, "%Y%m%d%H%M%S").timetuple())
           os.utime(file, (fileDate, fileDate))
           operationCounter += 1

    if filesToDelete:
        for file in filesToDelete:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            os.remove(file)
            operationCounter += 1

    if dirsToDelete:
        for dir in dirsToDelete:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            os.rmdir(dir)
            operationCounter += 1

    if filesToModifyLocal:
        for file in filesToModifyLocal:
            print 'COMMITING {0}/{1}'.format(operationCounter, totalOperations)
            os.remove(file)
            with open(file, 'wb') as ftpfile:
               ftp.retrbinary('RETR ' + unixPath(file), ftpfile.write)
            fileDate = ftp.sendcmd('MDTM ' + unixPath(file))[4:]
            fileDate = time.mktime(datetime.strptime(fileDate, "%Y%m%d%H%M%S").timetuple())
            os.utime(file, (fileDate, fileDate))
            operationCounter += 1
    return


def main(argv):
    # managing input arguments
    argParser = argparse.ArgumentParser(description = 'nfProjects FTP server synchronization tool.')
    argParser.add_argument('Mode', choices = ['i', 'p', 'u'],
                            help = "'i' is used to initialize the Data folder;"
                                + "'p' pushes current data from the directory to the server; "
                                + "'u' downloads data from server to repo, if needed")
    args = argParser.parse_args()

    # playing with paths
    dataPath = os.path.realpath(__file__)
    dataPath = os.path.join(dataPath, os.path.pardir, os.path.pardir)
    dataPath = os.path.normpath(dataPath + DATA_DIR)

    ftp.login("guest")
    os.chdir(dataPath)

    if args.Mode == 'i':
        InitMode()
    elif args.Mode == 'p':
        PushMode()
    else:
        UpdateMode()

    print '## DONE ##'
    return

    
if __name__ == '__main__':
    main(sys.argv)
