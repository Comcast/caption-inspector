cmake_minimum_required(VERSION 3.12)
project(caption-inspector C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(include)
include_directories(python)
include_directories(src)
include_directories(/Applications/Xcode.app/Contents/Developer//Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include)
include_directories(/usr/local/include)

add_executable(caption-inspector
        include/autodetect_file.h
        include/buffer_utils.h
        include/captions_file.h
        include/cc_data_output.h
        include/cc_utils.h
        include/context.h
        include/debug.h
        include/dtvcc_decode.h
        include/dtvcc_output.h
        include/line21_decode.h
        include/line21_output.h
        include/mcc_decode.h
        include/mcc_encode.h
        include/mcc_output.h
        include/mpeg_file.h
        include/output_utils.h
        include/pipeline_structs.h
        include/pipeline_utils.h
        include/external_adaptor.h
        include/scc_encode.h
        include/types.h
        src/sink/cc_data_output.c
        src/sink/dtvcc_output.c
        src/sink/line21_output.c
        src/sink/mcc_output.c
        src/source/mcc_file.c
        src/source/mov_file.c
        src/source/mpeg_file.c
        src/source/scc_file.c
        src/utils/autodetect_file.c
        src/utils/buffer_utils.c
        src/utils/cc_utils.c
        src/utils/debug.c
        src/utils/output_utils.c
        src/utils/pipeline_utils.c
        src/utils/external_adaptor.c
        src/utils/version.c
        src/xform/dtvcc_decode.c
        src/xform/line21_decode.c
        src/xform/mcc_decode.c
        src/xform/mcc_encode.c
        src/xform/scc_encode.c
        src/main.c)

add_executable(utest__buffer_utils_c
        include/cc_utils.h
        include/debug.h
        include/types.h
        src/utils/debug.c
        test/test_engine.c
        test/test_engine.h
        test/utest__buffer_utils.c)

add_executable(utest__cc_utils_c
        include/debug.h
        include/types.h
        src/utils/debug.c
        test/test_engine.c
        test/test_engine.h
        test/utest__cc_utils.c)

add_executable(utest__external_adaptor_c
        include/debug.h
        include/types.h
        src/utils/debug.c
        test/test_engine.c
        test/test_engine.h
        test/utest__external_adaptor.c)

add_executable(utest__output_utils_c
        include/debug.h
        include/types.h
        src/utils/debug.c
        test/test_engine.c
        test/test_engine.h
        test/utest__output_utils.c)

add_executable(utest__pipeline_utils_c
        include/debug.h
        include/types.h
        src/utils/debug.c
        test/test_engine.c
        test/test_engine.h
        test/utest__pipeline_utils.c)

add_executable(itest__buffer_utils
        include/debug.h
        include/types.h
        src/utils/debug.c
        src/utils/buffer_utils.c
        test/test_engine.c
        test/test_engine.h
        test/itest__buffers.c)

add_executable(itest__pipeline_utils
        include/debug.h
        include/types.h
        src/utils/debug.c
        src/utils/buffer_utils.c
        src/utils/cc_utils.c
        src/utils/pipeline_utils.c
        test/test_engine.c
        test/test_engine.h
        test/itest__pipeline.c)

set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/lib -lavformat -lavcodec -lavutil -lz -lgpac")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCOMPILE_GPAC")
