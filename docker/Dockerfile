FROM debian:bullseye

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
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

CMD ["bash"]