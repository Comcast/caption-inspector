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

RUN apt-get install -y git && pip install pytest && apt-get install -y mediainfo && cp /usr/bin/mediainfo /usr/local/bin/mediainfo && apt-get install -y npm && npm i -g xunit-viewer

# build /app/caption-inspector executable
WORKDIR /app
RUN git clone https://github.com/Comcast/caption-inspector.git; cd caption-inspector/test; make docker-test

