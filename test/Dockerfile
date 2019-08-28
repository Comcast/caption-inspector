FROM python:3.8-rc-buster

ENV FFMPEG_VERSION=4.0.2 LD_LIBRARY_PATH=/usr/local/lib

RUN apt-get update

# build ffmpeg libraries
RUN apt-get install -y make curl gcc g++ nasm yasm && \
  apt-get install -y opencl-dev vim libass-dev libavformat-dev \
  libavutil-dev libavfilter-dev uuid-dev zlib1g-dev && \
  DIR=$(mktemp -d) && cd ${DIR} && \
  curl -s http://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.gz | tar zxvf - -C . && \
  cd ffmpeg-${FFMPEG_VERSION} && \
  ./configure  --enable-version3 --enable-hardcoded-tables --enable-shared --enable-static \
    --enable-small --enable-libass --enable-postproc --enable-avresample --enable-libfreetype \
    --disable-lzma --enable-opencl --enable-pthreads && \
  make && \
  make install && \
  make distclean && \
  rm -rf ${DIR}

RUN apt-get install -y git git && apt-get install -y make && apt-get install -y clang && pip install pytest && apt-get install -y mediainfo && cp /usr/bin/mediainfo /usr/local/bin/mediainfo && apt-get install -y npm && npm i -g xunit-viewer

# pull and build gpac
RUN git clone https://github.com/Comcast/gpac-caption-extractor.git && \
    cp -r /gpac-caption-extractor/include/gpac /usr/local/include && \
    cd gpac-caption-extractor && make gpac && cp libgpac.so /usr/local/lib


# add directory /app/caption-inspector
WORKDIR /app
RUN mkdir caption-inspector

ENTRYPOINT /usr/bin/make -C /app/caption-inspector/test docker-test
