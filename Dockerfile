FROM ubuntu:22.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        gcc python3 python3-pip cmake cmake-data libraw-dev libopencv-dev \
        autoconf automake libtool pkg-config jasper libjpeg-dev liblcms2-dev \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir /source
VOLUME /source


RUN useradd -u 10000 mchapman
USER mchapman
WORKDIR /home/mchapman

ENTRYPOINT ["/bin/bash", "/source/docker_scripts/in_container/build.sh"]