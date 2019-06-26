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
#     |    |
#     |    +-----/Makefile (Convenience Makefile that just points to this makefile)
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
#     |    +--/Makefile  (This Makefile)
#     |        ^^^^^^^^
#     |     YOU ARE HERE!
#     |
#     +---/test/{C Test Code}
#     |    |
#     |    +-----/libci-test.dylib (Generated Dynamic Test Library for use with Python Tests)
#     |    |
#     |    +-----/python/{Python Test Code}
#     |    |
#     |    +-----/Makefile  (Test Makefile)
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

DYLIB_NAME = libci.1.0.0.dylib

UNAME := $(shell uname)

SRC_OBJS = mcc_file.o    \
           mpeg_file.o   \
           scc_file.o
SRC_OBJS_WITH_PATH = $(foreach SRC_OBJS, $(SRC_OBJS), source/$(SRC_OBJS))

XFORM_OBJS = dtvcc_decode.o   \
             mcc_decode.o     \
             mcc_encode.o     \
             line21_decode.o  \
             scc_encode.o
XFORM_OBJS_WITH_PATH = $(foreach XFORM_OBJS, $(XFORM_OBJS), xform/$(XFORM_OBJS))

SINK_OBJS = cc_data_output.o  \
            dtvcc_output.o    \
            line21_output.o   \
            mcc_output.o
SINK_OBJS_WITH_PATH = $(foreach SINK_OBJS, $(SINK_OBJS), sink/$(SINK_OBJS))

UTIL_OBJS = autodetect_file.o \
            buffer_utils.o    \
            cc_utils.o        \
            debug.o           \
            output_utils.o    \
            pipeline_utils.o  \
            external_adaptor.o\
            version.o
UTIL_OBJS_WITH_PATH = $(foreach UTIL_OBJS, $(UTIL_OBJS), utils/$(UTIL_OBJS))

OBJS = $(SRC_OBJS) $(XFORM_OBJS) $(SINK_OBJS) $(UTIL_OBJS)
OBJS_WITH_PATH = $(SRC_OBJS_WITH_PATH) $(XFORM_OBJS_WITH_PATH) $(SINK_OBJS_WITH_PATH) $(UTIL_OBJS_WITH_PATH)
OBJS_IN_OBJ_DIR = $(foreach OBJS, $(OBJS), ../obj/$(OBJS))

ifeq ($(UNAME), Linux)
C_FLAGS = -Wpedantic -Wno-deprecated-declarations -Wall -fno-strict-aliasing -fPIC -DPIC
SO_FLAGS = -shared -W -L/usr/local/lib -lavformat -lavcodec -lavutil
LD_FLAGS = -L/usr/local/lib -lavformat -lavcodec -lavutil -luuid -lm
else
C_FLAGS = -Wpedantic -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -DVERSION_FILE_PRESENT -Dfopen64=fopen -Dopen64=open -Dlseek64=lseek
SO_FLAGS = -dynamiclib -undefined suppress -flat_namespace -L/usr/local/lib -lavformat -lavcodec -lavutil
LD_FLAGS = -L/usr/local/lib -lavformat -lavcodec -lavutil
endif

-include ../obj/$(OBJS:.o=.d)

CI_EXECUTABLE = ../caption-inspector
CI_SHARED_LIB = ../python/$(DYLIB_NAME)

all: ${CI_EXECUTABLE} ${CI_SHARED_LIB}

${CI_EXECUTABLE}: $(OBJS_WITH_PATH) main.o
	@echo "\n*** Compiling Caption Inpsector Executible. ***"
	gcc -o ${CI_EXECUTABLE} ../obj/main.o $(OBJS_IN_OBJ_DIR) $(LD_FLAGS) -lz

${CI_SHARED_LIB}: $(OBJS_WITH_PATH)
	@echo "\n*** Compiling Caption Inspector Dynamic Library to interface with Python. ***"
	gcc $(SO_FLAGS) -o ${CI_SHARED_LIB} $(OBJS_IN_OBJ_DIR)

clean:
	@echo "*** Cleaning Target Files. ***"
	rm -f ${CI_EXECUTABLE} ${CI_SHARED_LIB}
	@echo "\n*** Cleaning Object Files. ***"
	for i in $(OBJS) ; do ( rm -f ../obj/$$i ) ; done
	rm -f ../obj/main.o
	@echo "\n*** Cleaning Dependency Files. ***"
	for i in $(OBJS:.o=.d) ; do ( rm -f ../obj/$$i ) ; done
	rm -f ../obj/main.d

%.o: %.c
	gcc -c $(C_FLAGS) -I ../include $*.c -o ../obj/$(notdir $*.o)
	gcc -MM -I ../include $*.c > ../obj/$(notdir $*.d)