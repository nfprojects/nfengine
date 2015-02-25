## @file   color.py
## @author mkulagowski (mkkulagowski@gmail.com)
## @brief  Module for multiplatform colored console output


from ctypes import *
from sys import platform as _platform


short = c_short
word = c_ushort


class Coord(Structure):
    """struct in wincon.h."""
    _fields_ = [
        ("X", short),
        ("Y", short)]


class SmallRect(Structure):
    """struct in wincon.h."""
    _fields_ = [
        ("Left", short),
        ("Top", short),
        ("Right", short),
        ("Bottom", short)]


class ConsoleScreenBufferInfo(Structure):
    """struct in wincon.h."""
    _fields_ = [
        ("dwSize", Coord),
        ("dwCursorPosition", Coord),
        ("wAttributes", word),
        ("srWindow", SmallRect),
        ("dwMaximumWindowSize", Coord)]


class Colorizer():
    def __init__(self):
        StdOutputHandleId = c_ulong(0xfffffff5)
        windll.Kernel32.GetStdHandle.restype = c_ulong
        self.std_output_hdl = windll.Kernel32.GetStdHandle(StdOutputHandleId)
        self.screenBufferInfo = ConsoleScreenBufferInfo()
        windll.Kernel32.GetConsoleScreenBufferInfo(self.std_output_hdl, byref(self.screenBufferInfo))
        self.defColor = self.screenBufferInfo.wAttributes
        self.winPalette = {
            "black": 0x00,
            "blue": 0x01,
            "green": 0x02,
            "cyan": 0x03,
            "red": 0x04,
            "magenta": 0x05,
            "yellow": 0x06,
            "white": 0x07}
        self.unixPalette = {
            "black": 0,
            "red": 1,
            "green": 2,
            "yellow": 3,
            "blue": 4,
            "magenta": 5,
            "cyan": 6,
            "white": 7}


    def getWinColor(self, color):
        return self.winPalette.get(color, self.defColor)


    def getUnixColor(self, color):
        return self.unixPalette.get(color, self.defColor)


    def printWin(self, text, fgColor, bgColor, isBold):
        # get fgColor
        if fgColor is not None:
            fgColor = self.getWinColor(fgColor)

        # get bgColor
        if bgColor is not None:
            bgColor = self.getWinColor(bgColor)
        else:
            bgColor = self.getWinColor('black')

        # get isBold
        if isBold is True:
            fgColor = fgColor | 0x08

        # calculate output color no.
        color = fgColor | (bgColor << 1)

        # color & print
        windll.Kernel32.SetConsoleTextAttribute(self.std_output_hdl, color)
        print text,
        windll.Kernel32.SetConsoleTextAttribute(self.std_output_hdl, self.defColor)


    def printUnix(self, text, fgColor, bgColor, isBold):
        color = '\033['

        # get fgColor
        if fgColor is not None:
            fgColor = self.getUnixColor(fgColor)

        # get bgColor
        if bgColor is not None:
            bgColor = self.getUnixColor(bgColor)

        # get isBold
        if isBold is True:
            color += '1;'
        else:
            color += '0;'

        # calculate output color no.
        color += '3' + fgColor
        if bgColor is not None:
            color += ';4' + bgColor
        color += 'm'

        # color & print
        print format.color(text),


    def printMulti(self, text, fgColor, bgColor, isBold):
        if _platform == "linux" or _platform == "linux2":
            self.printUnix(text, fgColor, bgColor, isBold)
        elif _platform == "win32":
            self.printWin(text, fgColor, bgColor, isBold)
        else:
            print text,
