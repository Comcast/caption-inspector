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
           mov_file.o    \
           mpeg_file.o   \
           scc_file.o
SRC_OBJS_WITH_PATH = $(foreach SRC_OBJS, $(SRC_OBJS), source/$(SRC_OBJS))

XFORM_OBJS = dtvcc_decode.o   \
             mcc_decode.o     \
             mcc_encode.o     \
             line21_decode.o  \
             scc_encode.o     \
             smpte_encode.o
XFORM_OBJS_WITH_PATH = $(foreach XFORM_OBJS, $(XFORM_OBJS), xform/$(XFORM_OBJS))

SINK_OBJS = cc_data_output.o  \
            dtvcc_output.o    \
            line21_output.o   \
            mcc_output.o      \
            smpte_output.o
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
SO_FLAGS = -shared -W -L/usr/local/lib -lavformat -lavcodec -lavutil -luuid
LD_FLAGS = -L/usr/local/lib -luuid -lm -lz
LD_FLAGS_FFMPEG = -lavformat -lavcodec -lavutil
else
C_FLAGS = -Wpedantic -Wno-deprecated-declarations
SO_FLAGS = -dynamiclib -undefined suppress -flat_namespace -L/usr/local/lib -lavformat -lavcodec -lavutil
LD_FLAGS = -L/usr/local/lib -lz
LD_FLAGS_FFMPEG = -lavformat -lavcodec -lavutil
endif

-include ../obj/$(OBJS:.o=.d)

CI_EXECUTABLE = ../caption-inspector
CI_SHARED_LIB = ../python/$(DYLIB_NAME)

all: ${CI_EXECUTABLE} ${CI_SHARED_LIB}

${CI_EXECUTABLE}: $(OBJS_WITH_PATH) main.o
	@echo "\n*** Compiling Caption Inpsector Executible. ***"
	clang -o ${CI_EXECUTABLE} ../obj/main.o $(OBJS_IN_OBJ_DIR) $(LD_FLAGS) $(LD_FLAGS_FFMPEG)

no_ffmpeg: $(OBJS_WITH_PATH) main.o
	@echo "\n*** Compiling Caption Inpsector Executible without FFMPEG. ***"
	clang -o ${CI_EXECUTABLE} ../obj/main.o $(OBJS_IN_OBJ_DIR) $(LD_FLAGS)

/usr/local/lib/libgpac.so:
	@echo "Building MOV Support requires GPAC - $@"
	@echo "To build GPAC for consumption by Caption Inspector you need the following repo: https://github.com/Comcast/gpac"
	@echo "Build that repo with `make install`, which will add the shared library to /usr/local/lib and add the"
	@echo "appropriate headers to /usr/local/include. Once that is done you can build Caption Inspector with MOV Support"
	@exit 1

with_gpac: /usr/local/lib/libgpac.so $(OBJS_WITH_PATH) main.o
	@echo "\n*** Compiling Caption Inpsector Executible with GPAC. ***"
	clang -o ${CI_EXECUTABLE} ../obj/main.o $(OBJS_IN_OBJ_DIR) $(LD_FLAGS) $(LD_FLAGS_FFMPEG) -lgpac

ci_without_ffmpeg: C_FLAGS:=$(C_FLAGS) -DDONT_COMPILE_FFMPEG
ci_without_ffmpeg: no_ffmpeg

ci_with_gpac: C_FLAGS:=$(C_FLAGS) -DCOMPILE_GPAC
ci_with_gpac: with_gpac

${CI_SHARED_LIB}: $(OBJS_WITH_PATH)
	@echo "\n*** Compiling Caption Inspector Dynamic Library to interface with Python. ***"
	clang $(SO_FLAGS) -o ${CI_SHARED_LIB} $(OBJS_IN_OBJ_DIR)

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
	@if ! [ -d "../obj" ]; then mkdir ../obj; fi
	clang -c $(C_FLAGS) -I ../include $*.c -o ../obj/$(notdir $*.o)
	clang -MM $(C_FLAGS) -I ../include $*.c > ../obj/$(notdir $*.d)
