#!/usr/bin/env python
# -*- coding: latin-1 -*- 

import ctypes
import os
import sys
import subprocess
from datetime import datetime

CAPTION_INSPECTOR_EXE = '../../caption-inspector'
CAPTION_INSPECTOR_LIBRARY = './libci-test.1.0.0.dylib'
OUTPUT_FILENAME = 'compiled_output.xml'

py_test_suites = ['autodetect_file', 'external_adaptor', 'output_utils']
py_test_suites_names = {'autodetect_file': 'Test Suite: Autodetect File',
                        'external_adaptor': 'Test Suite: External Adaptor',
                        'output_utils': 'Test Suite: Output Utilities'}
c_integ_test_suites = ['itest__buffer_utils', 'itest__pipeline_utils']
c_unit_test_suites = ['utest__buffer_utils_c', 'utest__cc_utils_c', 'utest__external_adaptor_c',
                      'utest__output_utils_c', 'utest__pipeline_utils_c']


def run_test(test_name):
    os.system('pytest -o junit_suite_name=' + test_name + ' --junitxml ' + test_name + '.xml test__' + test_name + '.py')
    print('Testing Complete: Results written to file - ' + test_name + '.xml')
#    os.system('xunit-viewer --results=' + test_name + '.xml --output=' + test_name + '.html --title="Caption Converter Python Test Suite"')
#    print('XML Results file: ' + test_name + '.xml converted to HTML: ' + test_name + '.html')
#    os.system('open ' + test_name + '.html')


if __name__ == "__main__":
    if len(sys.argv) is 2:
        run_test(str(sys.argv[1]))
    else:
        exe_ver = '???'
        lib_ver = '???'
        response = subprocess.check_output([CAPTION_INSPECTOR_EXE, '-v'], stderr=subprocess.STDOUT)
        if response.split()[0] == b"Version:":
            exe_ver = response.strip().decode('utf-8')
        clib = ctypes.CDLL(CAPTION_INSPECTOR_LIBRARY)
        clib.ExtrnlAdptrGetVersion.restype = ctypes.c_char_p
        response = clib.ExtrnlAdptrGetVersion()
        lib_ver = ctypes.c_char_p(response).value.decode('utf-8')
        if exe_ver != lib_ver:
            print("Version Mismatch! " + exe_ver + " vs. " + lib_ver)
            exit(1)
        date_str = str(datetime.now().strftime('%Y_%m_%d__%H_%M_%S'))
        out_file_name = date_str + '_test_output'
        file = open(out_file_name + '.xml', "w")
        file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
        file.write("<testsuites name=\"Caption Converter " + exe_ver + " Tests\">\n")
        file.close()
        for test in py_test_suites:
            os.system('pytest -o junit_suite_name="' + py_test_suites_names[test] + '" --junitxml ' + date_str + '_' + test + '.xml test__' + test + '.py')
            os.system('cat ' + date_str + '_' + test + '.xml >> ' + out_file_name + '.xml')
#            os.system('rm ' + date_str + '_' + test + '.xml')
        for test in c_integ_test_suites:
            os.system('../' + test + ' ' + date_str + '_' + test + '.xml')
            os.system('cat ' + date_str + '_' + test + '.xml >> ' + out_file_name + '.xml')
#            os.system('rm ' + date_str + '_' + test + '.xml')
        for test in c_unit_test_suites:
            os.system('../' + test + ' ' + date_str + '_' + test + '.xml')
            os.system('cat ' + date_str + '_' + test + '.xml >> ' + out_file_name + '.xml')
#            os.system('rm ' + date_str + '_' + test + '.xml')
        os.system("echo \"</testsuites>\n\n\" >> " + out_file_name + '.xml')
        print('xunit-viewer --results=' + out_file_name + '.xml --output=' + out_file_name +
              '.html --title="Caption Converter ' + exe_ver + ' Full Test Suite"')
        os.system('xunit-viewer --results=' + out_file_name + '.xml --output=' + out_file_name +
                  '.html --title="Caption Converter ' + exe_ver + ' Full Test Suite"')
        print('XML Results file: ' + out_file_name + '.xml converted to HTML: ' + out_file_name + '.html')
        os.system('open ' + out_file_name + '.html')

