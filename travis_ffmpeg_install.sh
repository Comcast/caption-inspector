#!/usr/bin/env bash

# This Script is only to be used by Travis CI for installing FFMPEG on its VM to build.
# It should not be run from the command line and is not needed to build outside of Travis.

FFMPEG_VERSION=4.0.2
#export LD_LIBRARY_PATH=/usr/local/lib
apt-get update
apt-get install -y make curl gcc g++ nasm yasm
apt-get install -y ocl-icd-opencl-dev libass-dev libavformat-dev libavutil-dev libavfilter-dev uuid-dev zlib1g-dev && \
  DIR=$(mktemp -d) && cd ${DIR} && \
  curl -s http://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.gz | tar zxvf - -C . && \
  cd ffmpeg-${FFMPEG_VERSION} && \
  ./configure  --enable-version3 --enable-hardcoded-tables --enable-shared --enable-static \
    --enable-small --enable-libass --enable-postproc --enable-avresample --enable-libfreetype \
    --disable-lzma --enable-opencl --enable-pthreads
make
make install
