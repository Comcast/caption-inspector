################################################################################
#                                                                              #
#                            M A K E F I L E                                   #
#                                                                              #
################################################################################

################################################################################
#
# Directory Structure:
#    /CaptionInspector/
#     |
#     +---/include/{All Include Files}
#     |
#     +---/obj/{All Generated Object and Dependency Files}
#     |
#     +---/python/{Python Code}
#     |    |
#     |    +-----/libci.dylib (Generated Dynamic Library for use with Python)
#     |
#     +---/src/
#     |    |
#     |    +--/source/{Source Files for the Sources}
#     |    |
#     |    +--/xform/{Source Files for the Transforms}
#     |    |
#     |    +--/sink/{Source Files for the Sinks}
#     |    |
#     |    +--/utils/{Source Files for the Utilities}
#     |    |
#     |    +--/Makefile  (Executible Makefile)
#     |
#     +---/test/{C Test Code}
#     |    |
#     |    +-----/libci-test.dylib (Generated Dynamic Test Library for use with Python Tests)
#     |    |
#     |    +-----/python/{Python Test Code}
#     |    |      |
#     |    |      +-----/Makefile  (Run Test Makefile)
#     |    |
#     |    +-----/Makefile  (This Makefile)
#     |           ^^^^^^^^
#     |        YOU ARE HERE!
#     |
#     +---/Makefile (Convenience Makefile that just points to this makefile)
#     |
#     +---/caption-inspector  (Generated Binary Executable)
#
# Build Process:
#     This makefile will build the dependency files, and the objects for all
#     of the source in the repository. It will then complile all of the objects
#     into a binary executable that resides up one folder and also into a
#     dynamic library, which resides in the python directory for use with the
#     python code.
#
# Adding a New Source File:
#     To add a new file to an existing sub folder, simply add the .o to the
#     list of object files for that folder. e.g. Adding a file to the 'xform'
#     folder:
#              XFORM_OBJS = dtvcc_decode.o   \
#                           mcc_decode.o     \
#  New File --->            foo.o            \
#                           sei_decode.o     \
#                           line21_decode.o  \
#                           scc_encode.o     \
#                           sei_encode.o
#              
# Adding a New Source Sub Folder:
#     To add a new sub folder you need to follow the pattern add the new folder
#     to the following:
#              NEW_OBJS = foo.o   \
#                         bar.0
#              NEW_OBJS_WITH_PATH = $(foreach NEW_OBJS, $(NEW_OBJS), source/$(NEW_OBJS))
#
#
#              OBJS = $(SRC_OBJS) $(XFORM_OBJS) $(SINK_OBJS) $(UTIL_OBJS) $(NEW_OBJS) <--- Also Add This
#              OBJS_WITH_PATH = $(SRC_OBJS_WITH_PATH) $(XFORM_OBJS_WITH_PATH) $(SINK_OBJS_WITH_PATH) $(UTIL_OBJS_WITH_PATH) $(NEW_OBJS) <--- And This
#
#
################################################################################

UNAME := $(shell uname)

SRC_OBJS = mcc_file.o    \
           mpeg_file.o   \
           scc_file.o
SRC_OBJS_WITH_PATH = $(foreach SRC_OBJS, $(SRC_OBJS), ../src/source/$(SRC_OBJS))

XFORM_OBJS = dtvcc_decode.o   \
             mcc_decode.o     \
             mcc_encode.o     \
             line21_decode.o  \
             scc_encode.o
XFORM_OBJS_WITH_PATH = $(foreach XFORM_OBJS, $(XFORM_OBJS), ../src/xform/$(XFORM_OBJS))

SINK_OBJS = cc_data_output.o  \
            dtvcc_output.o    \
            line21_output.o   \
            mcc_output.o
SINK_OBJS_WITH_PATH = $(foreach SINK_OBJS, $(SINK_OBJS), ../src/sink/$(SINK_OBJS))

UTIL_OBJS = autodetect_file.o \
            buffer_utils.o    \
            cc_utils.o        \
            debug.o           \
            output_utils.o    \
            pipeline_utils.o  \
            external_adaptor.o\
            version.o
UTIL_OBJS_WITH_PATH = $(foreach UTIL_OBJS, $(UTIL_OBJS), ../src/utils/$(UTIL_OBJS))

OBJS = $(SRC_OBJS) $(XFORM_OBJS) $(SINK_OBJS) $(UTIL_OBJS)
OBJS_WITH_PATH = $(SRC_OBJS_WITH_PATH) $(XFORM_OBJS_WITH_PATH) $(SINK_OBJS_WITH_PATH) $(UTIL_OBJS_WITH_PATH)
OBJS_IN_OBJ_DIR = $(foreach OBJS, $(OBJS), ../obj/$(OBJS))

ifeq ($(UNAME), Linux)
C_FLAGS = -Wpedantic -Wno-deprecated-declarations -Wall -fno-strict-aliasing -fPIC -DPIC
SO_FLAGS = -shared -W -L/usr/local/lib -lavformat -lavcodec -lavutil -luuid
LD_FLAGS = -L/usr/local/lib -lavformat -lavcodec -lavutil -luuid -lm
else
C_FLAGS = -Wpedantic -Wno-deprecated-declarations
SO_FLAGS = -dynamiclib -undefined suppress -flat_namespace -L/usr/local/lib -lavformat -lavcodec -lavutil
LD_FLAGS = -L/usr/local/lib -lavformat -lavcodec -lavutil -lz
endif

-include ../obj/$(OBJS:.o=.d)

PYTEST_SHARED_LIB = libci-test.1.0.0.dylib

TEST_OBJS = debug.o test_engine.o
TEST_OBJS_WITH_PATH = ../src/utils/debug.o test_engine.o
TEST_OBJS_IN_OBJ_DIR = $(foreach TEST_OBJS, $(TEST_OBJS), ../obj/$(TEST_OBJS))

PYTEST_OBJS = python_test_shim.o
PYTEST_OBJS_WITH_PATH = python_test_shim.o
PYTEST_OBJS_IN_OBJ_DIR = $(foreach PYTEST_OBJS, $(PYTEST_OBJS), ../obj/$(PYTEST_OBJS))

UTEST_BUFFER_UTILS_EXE = utest__buffer_utils_c
UTEST_BUFFER_UTILS_OBJ = utest__buffer_utils.o
UTEST_BUFFER_UTILS_OBJ_IN_OBJ_DIR = ../obj/utest__buffer_utils.o
UTEST_CC_UTILS_EXE = utest__cc_utils_c
UTEST_CC_UTILS_OBJ = utest__cc_utils.o
UTEST_CC_UTILS_OBJ_IN_OBJ_DIR = ../obj/utest__cc_utils.o
UTEST_EXTERNAL_ADAPTOR_EXE = utest__external_adaptor_c
UTEST_EXTERNAL_ADAPTOR_OBJ = utest__external_adaptor.o
UTEST_EXTERNAL_ADAPTOR_OBJ_IN_OBJ_DIR = ../obj/utest__external_adaptor.o
UTEST_OUTPUT_UTILS_EXE = utest__output_utils_c
UTEST_OUTPUT_UTILS_OBJ = utest__output_utils.o
UTEST_OUTPUT_UTILS_OBJ_IN_OBJ_DIR = ../obj/utest__output_utils.o
UTEST_PIPELINE_UTILS_EXE = utest__pipeline_utils_c
UTEST_PIPELINE_UTILS_OBJ = utest__pipeline_utils.o
UTEST_PIPELINE_UTILS_OBJ_IN_OBJ_DIR = ../obj/utest__pipeline_utils.o

UNIT_TESTS = $(UTEST_BUFFER_UTILS_EXE) $(UTEST_CC_UTILS_EXE) $(UTEST_EXTERNAL_ADAPTOR_EXE) $(UTEST_OUTPUT_UTILS_EXE) $(UTEST_PIPELINE_UTILS_EXE)
UTEST_OBJS = $(UTEST_BUFFER_UTILS_OBJ) $(UTEST_CC_UTILS_OBJ) $(UTEST_EXTERNAL_ADAPTOR_OBJ) $(UTEST_OUTPUT_UTILS_OBJ) $(UTEST_PIPELINE_UTILS_OBJ)

ITEST_BUFFER_UTILS_EXE = itest__buffer_utils
ITEST_BUFFER_UTILS_OBJ = buffer_utils.o itest__buffers.o
ITEST_BUFFER_UTILS_WITH_PATH = ../src/utils/buffer_utils.o itest__buffers.o
ITEST_BUFFER_UTILS_IN_OBJ_DIR = $(foreach ITEST_BUFFER_UTILS_OBJ, $(ITEST_BUFFER_UTILS_OBJ), ../obj/$(ITEST_BUFFER_UTILS_OBJ))
ITEST_PIPELINE_UTILS_EXE = itest__pipeline_utils
ITEST_PIPELINE_UTILS_OBJ = buffer_utils.o cc_utils.o pipeline_utils.o itest__pipeline.o
ITEST_PIPELINE_UTILS_WITH_PATH = ../src/utils/buffer_utils.o ../src/utils/cc_utils.o ../src/utils/pipeline_utils.o itest__pipeline.o
ITEST_PIPELINE_UTILS_IN_OBJ_DIR = $(foreach ITEST_PIPELINE_UTILS_OBJ, $(ITEST_PIPELINE_UTILS_OBJ), ../obj/$(ITEST_PIPELINE_UTILS_OBJ))

INTEG_TESTS = $(ITEST_BUFFER_UTILS_EXE) $(ITEST_PIPELINE_UTILS_EXE)
ITEST_OBJS = ${ITEST_BUFFER_UTILS_OBJ} ${ITEST_PIPELINE_UTILS_OBJ}

all: ${PYTEST_SHARED_LIB} $(UNIT_TESTS) $(INTEG_TESTS)

${PYTEST_SHARED_LIB}: $(OBJS_WITH_PATH) $(PYTEST_OBJS_WITH_PATH)
	@echo "\n*** Compiling Caption Inspector Test Dynamic Library to interface with Python Test Drivers. ***"
	gcc $(SO_FLAGS) -o python/${PYTEST_SHARED_LIB} $(OBJS_IN_OBJ_DIR) ${PYTEST_OBJS_IN_OBJ_DIR}

${ITEST_PIPELINE_UTILS_EXE}: ${ITEST_PIPELINE_UTILS_WITH_PATH} ${TEST_OBJS_WITH_PATH}
	gcc -o ${ITEST_PIPELINE_UTILS_EXE} ${ITEST_PIPELINE_UTILS_IN_OBJ_DIR} ${TEST_OBJS_IN_OBJ_DIR}

${ITEST_BUFFER_UTILS_EXE}: ${ITEST_BUFFER_UTILS_WITH_PATH} ${TEST_OBJS_WITH_PATH}
	gcc -o ${ITEST_BUFFER_UTILS_EXE} ${ITEST_BUFFER_UTILS_IN_OBJ_DIR} ${TEST_OBJS_IN_OBJ_DIR}

${UTEST_BUFFER_UTILS_EXE}: ${UTEST_BUFFER_UTILS_OBJ} ${TEST_OBJS_WITH_PATH}
	gcc -o ${UTEST_BUFFER_UTILS_EXE} ${UTEST_BUFFER_UTILS_OBJ_IN_OBJ_DIR} ${TEST_OBJS_IN_OBJ_DIR}

${UTEST_CC_UTILS_EXE}: ${UTEST_CC_UTILS_OBJ} ${TEST_OBJS_WITH_PATH}
	gcc -o ${UTEST_CC_UTILS_EXE} ${UTEST_CC_UTILS_OBJ_IN_OBJ_DIR} ${TEST_OBJS_IN_OBJ_DIR}

${UTEST_EXTERNAL_ADAPTOR_EXE}: ${UTEST_EXTERNAL_ADAPTOR_OBJ} ${TEST_OBJS_WITH_PATH}
	gcc -o ${UTEST_EXTERNAL_ADAPTOR_EXE} ${UTEST_EXTERNAL_ADAPTOR_OBJ_IN_OBJ_DIR} ${TEST_OBJS_IN_OBJ_DIR}

${UTEST_OUTPUT_UTILS_EXE}: ${UTEST_OUTPUT_UTILS_OBJ} ${TEST_OBJS_WITH_PATH}
	gcc -o ${UTEST_OUTPUT_UTILS_EXE} ${UTEST_OUTPUT_UTILS_OBJ_IN_OBJ_DIR} ${TEST_OBJS_IN_OBJ_DIR}

${UTEST_PIPELINE_UTILS_EXE}: ${UTEST_PIPELINE_UTILS_OBJ} ${TEST_OBJS_WITH_PATH}
	gcc -o ${UTEST_PIPELINE_UTILS_EXE} ${UTEST_PIPELINE_UTILS_OBJ_IN_OBJ_DIR} ${TEST_OBJS_IN_OBJ_DIR}

test:
	@echo "\n*** Cleaning for Test ***"
	make clean
	cd ../; make clean
	@echo "\n*** Building for Test ***"
	make all
	cd ../; make all
	@echo "\n*** Running Tests ***"
	cd python; python3 test_engine.py

clean:
	@echo "\n*** Cleaning Target Files. ***"
	rm -f python/${PYTEST_SHARED_LIB}
	for i in $(UNIT_TESTS) ; do ( rm -f $$i ) ; done
	for i in $(INTEG_TESTS) ; do ( rm -f $$i ) ; done
	@echo "\n*** Cleaning Object Files. ***"
	for i in $(OBJS) ; do ( rm -f ../obj/$$i ) ; done
	for i in $(TEST_OBJS) ; do ( rm -f ../obj/$$i ) ; done
	for i in $(UTEST_OBJS) ; do ( rm -f ../obj/$$i ) ; done
	for i in $(ITEST_OBJS) ; do ( rm -f ../obj/$$i ) ; done
	for i in $(PYTEST_OBJS) ; do ( rm -f ../obj/$$i ) ; done
	@echo "\n*** Cleaning Dependency Files. ***"
	for i in $(OBJS:.o=.d) ; do ( rm -f ../obj/$$i ) ; done
	for i in $(UTEST_OBJS:.o=.d) ; do ( rm -f ../obj/$$i ) ; done
	for i in $(ITEST_OBJS:.o=.d) ; do ( rm -f ../obj/$$i ) ; done
	for i in $(PYTEST_OBJS:.o=.d) ; do ( rm -f ../obj/$$i ) ; done
	@echo "\n*** Cleaning Test Artifacts. ***"
	rm -f python/*.xml
	rm -f python/*.html

%.o: %.c
	@if ! [ -d "../obj" ]; then mkdir ../obj; fi
	gcc -c $(C_FLAGS) -I ../include $*.c -o ../obj/$(notdir $*.o)
	gcc -MM -I ../include $*.c > ../obj/$(notdir $*.d)
