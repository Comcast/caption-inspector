#!/usr/bin/env python
# -*- coding: latin-1 -*- 

import ctypes
import difflib
import os

CAPTION_CONVERTER_LIBRARY = './libcttp-test.1.0.0.dylib'


def compare_text(first, second, ctr, ignore):
    if ctr in ignore:
        return True
    retval = True
    for loop in range(len(first)):
        if first[loop] != second[loop]:
            retval = False
            print("Different [Line-%d]: " % ctr)
            d = difflib.Differ()
            diff = d.compare(first[loop], second[loop])
            print("".join(diff))
    return retval


def compare_files(generated_file, master_file, ignore):
    print('Comparing: %s vs. %s' % (generated_file.name, master_file.name))
    counter = 0
    generated_line = generated_file.readline()
    while generated_line:
        counter = counter + 1
        master_line = master_file.readline()
        identical = compare_text(generated_line, master_line, counter, ignore)
        assert identical is True
        generated_line = generated_file.readline()


class TestClass(object):
    def test__EA_Establish_SCC_Pipeline(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        if os.path.exists('./tmp') is not True:
            os.mkdir('tmp')
        retval = clib.ExtrnlAdptrPlumbSccPipeline('../media/Plan9fromOuterSpace.scc'.encode('utf-8'),
                                                  './tmp/Plan9fromOuterSpace'.encode('utf-8'), 2400)
        assert retval is 1
        clib.ExtrnlAdptrDriveDecodePipeline()
        assert os.path.exists('./tmp/Plan9fromOuterSpace-C1.608'.encode('utf-8')) is True
        generated_file = open("./tmp/Plan9fromOuterSpace-C1.608", "r")
        master_file = open("../media/Plan9fromOuterSpace-C1.608", "r")
        compare_files(generated_file, master_file, [1])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/Plan9fromOuterSpace.mcc'.encode('utf-8')) is True
        generated_file = open("./tmp/Plan9fromOuterSpace.mcc", "r")
        master_file = open("../media/Plan9fromOuterSpace.mcc", "r")
        compare_files(generated_file, master_file, [41,43,44])
        generated_file.close()
        master_file.close()
        os.remove("./tmp/Plan9fromOuterSpace-C1.608")
        os.remove("./tmp/Plan9fromOuterSpace.mcc")
        os.remove("./tmp/Plan9fromOuterSpace.ccd")
        os.removedirs("./tmp")

    def test__EA_Establish_MCC_Pipeline(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        if os.path.exists('./tmp') is not True:
            os.mkdir('tmp')
        retval = clib.ExtrnlAdptrPlumbMccPipeline('../media/NightOfTheLivingDead.mcc'.encode('utf-8'),
                                                  './tmp/NightOfTheLivingDead'.encode('utf-8'))
        assert retval is 1
        clib.ExtrnlAdptrDriveDecodePipeline()
        assert os.path.exists('./tmp/NightOfTheLivingDead-C1.608'.encode('utf-8')) is True
        generated_file = open("./tmp/NightOfTheLivingDead-C1.608", "r")
        master_file = open("../media/NightOfTheLivingDead-C1.608", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/NightOfTheLivingDead-S1.708'.encode('utf-8')) is True
        generated_file = open("./tmp/NightOfTheLivingDead-S1.708", "r")
        master_file = open("../media/NightOfTheLivingDead-S1.708", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        os.remove("./tmp/NightOfTheLivingDead-C1.608")
        os.remove("./tmp/NightOfTheLivingDead-S1.708")
        os.remove("./tmp/NightOfTheLivingDead.ccd")
        os.removedirs("./tmp")

    def test__EA_Establish_MPG_Pipeline_w_Artifacts(self):
        clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
        if os.path.exists('./tmp') is not True:
            os.mkdir('tmp')
        retval = clib.ExtrnlAdptrPlumbMpegPipeline('../media/BigBuckBunny_256x144-24fps.ts'.encode('utf-8'),
                                                   './tmp/BigBuckBunny_256x144-24fps'.encode('utf-8'), 1,
                                                   './tmp/BigBuckBunny_256x144-24fps'.encode('utf-8'))
        assert retval is 1
        clib.ExtrnlAdptrDriveDecodePipeline()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps-C1.608'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps-C1.608", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps-C1.608", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps-C3.608'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps-C3.608", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps-C3.608", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps-S1.708'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps-S1.708", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps-S1.708", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps-S2.708'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps-S2.708", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps-S2.708", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps-S3.708'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps-S3.708", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps-S3.708", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps-S4.708'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps-S4.708", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps-S4.708", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps-S5.708'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps-S5.708", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps-S5.708", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps-S6.708'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps-S6.708", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps-S6.708", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps.ccd'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps.ccd", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps.ccd", "r")
        compare_files(generated_file, master_file, [35, 36])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps.inf'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps.inf", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps.inf", "r")
        compare_files(generated_file, master_file, [])
        generated_file.close()
        master_file.close()
        assert os.path.exists('./tmp/BigBuckBunny_256x144-24fps.mcc'.encode('utf-8')) is True
        generated_file = open("./tmp/BigBuckBunny_256x144-24fps.mcc", "r")
        master_file = open("../media/BigBuckBunny_256x144-24fps.mcc", "r")
        compare_files(generated_file, master_file, [41, 43, 44])
        generated_file.close()
        master_file.close()
        os.remove("./tmp/BigBuckBunny_256x144-24fps-C1.608")
        os.remove("./tmp/BigBuckBunny_256x144-24fps-C3.608")
        os.remove("./tmp/BigBuckBunny_256x144-24fps-S1.708")
        os.remove("./tmp/BigBuckBunny_256x144-24fps-S2.708")
        os.remove("./tmp/BigBuckBunny_256x144-24fps-S3.708")
        os.remove("./tmp/BigBuckBunny_256x144-24fps-S4.708")
        os.remove("./tmp/BigBuckBunny_256x144-24fps-S5.708")
        os.remove("./tmp/BigBuckBunny_256x144-24fps-S6.708")
        os.remove("./tmp/BigBuckBunny_256x144-24fps.ccd")
        os.remove("./tmp/BigBuckBunny_256x144-24fps.inf")
        os.remove("./tmp/BigBuckBunny_256x144-24fps.mcc")
        os.removedirs("./tmp")


if __name__ == "__main__":
    TestClass().test__EA_Establish_SCC_Pipeline()
    TestClass().test__EA_Establish_MCC_Pipeline()
    TestClass().test__EA_Establish_MPG_Pipeline_w_Artifacts()
