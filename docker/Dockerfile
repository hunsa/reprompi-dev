FROM debian:bookworm

# Docker file for ReproMPI (dev)

LABEL maintainer="Sascha Hunold <sascha@hunoldscience.net>"

RUN apt-get clean \
 && apt-get update \
 && apt-get install -y \
    curl \
    git \
    make \
    cmake \
    vim \
    wget \
    gcc \
    gdb \
    valgrind \
    cmake-curses-gui \
    libgsl-dev \
    mpich \
    build-essential \
    tmux \
    flex \
    python3 \
    autoconf \
    automake \
    libtool \
    libev-dev \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

RUN addgroup --gid 1000  sascha
RUN useradd -rm -d /home/sascha -s /bin/bash -g sascha -G sudo -u 1001 sascha
USER sascha
WORKDIR /home/sascha

CMD ["bash"]
