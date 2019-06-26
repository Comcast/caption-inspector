#!/usr/bin/env python
# -*- coding: latin-1 -*- 

import ctypes
import os
import pytest

CAPTION_CONVERTER_LIBRARY = './libcttp-test.1.0.0.dylib'

UNK_CAPTIONS_FILE = 0
SCC_CAPTIONS_FILE = 1
MCC_CAPTIONS_FILE = 2
MPEG_BINARY_FILE = 3


class TestClass(object):
    def test__Determine_SCC_File(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        file_type = clib.DetermineFileType("../media/Plan9fromOuterSpace.scc".encode('utf-8'))
        assert file_type is SCC_CAPTIONS_FILE

    def test__Determine_MCC_File(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        file_type = clib.DetermineFileType("../media/NightOfTheLivingDead.mcc".encode('utf-8'))
        assert file_type is MCC_CAPTIONS_FILE

    def test__Determine_MPG_File(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        file_type = clib.DetermineFileType("../media/BigBuckBunny_160x90-24fps.mpg".encode('utf-8'))
        assert file_type is MPEG_BINARY_FILE

    def test__Determine_TS_File(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        file_type = clib.DetermineFileType("../media/BigBuckBunny_256x144-24fps.ts".encode('utf-8'))
        assert file_type is MPEG_BINARY_FILE

    def test__Determine_UNK_File(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        file_type = clib.DetermineFileType("../media/LoremIpsum.txt".encode('utf-8'))
        assert file_type is UNK_CAPTIONS_FILE

    def test__Determine_DF_NDF(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        is_drop_frame = ctypes.c_ubyte(0)
        successful = clib.DetermineDropFrame("../media/BigBuckBunny_160x90-24fps.mpg".encode('utf-8'), 0,
                                             0, ctypes.byref(is_drop_frame))
        assert successful is 1
        assert is_drop_frame.value is 0

    def test__Determine_DF_IDF(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        is_drop_frame = ctypes.c_ubyte(0)
        successful = clib.DetermineDropFrame("../media/BigBuckBunny_160x90-24fps.mov".encode('utf-8'), 0,
                                             0, ctypes.byref(is_drop_frame))
        assert successful is 0
        successful = clib.DetermineDropFrame("../media/BigBuckBunny_256x144-24fps.ts".encode('utf-8'), 0,
                                             0, ctypes.byref(is_drop_frame))
        assert successful is 0

    def test__Determine_DF_Report(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        is_drop_frame = ctypes.c_ubyte(0)
        successful = clib.DetermineDropFrame("../media/BigBuckBunny_160x90-24fps.mpg".encode('utf-8'), 1,
                                             "./BigBuck".encode('utf-8'), ctypes.byref(is_drop_frame))
        assert os.path.isfile("./BigBuck.inf") is True
        assert successful is 1
        assert is_drop_frame.value is 0
        if os.path.isfile("./BigBuck.inf"):
            os.remove("./BigBuck.inf")
            assert os.path.isfile("./BigBuck.inf") is False

    def test__Determine_DF_Report_NULL(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        is_drop_frame = ctypes.c_ubyte(0)
        successful = clib.DetermineDropFrame("../media/BigBuckBunny_160x90-24fps.mpg".encode('utf-8'), 1,
                                             0, ctypes.byref(is_drop_frame))
        assert os.path.isfile("../media/BigBuckBunny_160x90-24fps.inf") is True
        assert successful is 1
        assert is_drop_frame.value is 0
        if os.path.isfile("../media/BigBuckBunny_160x90-24fps.inf"):
            os.remove("../media/BigBuckBunny_160x90-24fps.inf")
            assert os.path.isfile("../media/BigBuckBunny_160x90-24fps.inf") is False


if __name__ == "__main__":
    TestClass().test__Determine_SCC_File()
    TestClass().test__Determine_MCC_File()
    TestClass().test__Determine_MPG_File()
    TestClass().test__Determine_TS_File()
    TestClass().test__Determine_UNK_File()
    TestClass().test__Determine_DF_NDF()
    TestClass().test__Determine_DF_IDF()
    TestClass().test__Determine_DF_Report()
    TestClass().test__Determine_DF_Report_NULL()
