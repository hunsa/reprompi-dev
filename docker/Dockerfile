FROM debian:bullseye

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

CMD ["bash"]