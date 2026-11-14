# Cross-compilation environment for the busOBC (GomSpace NanoCom SDR MK3).
#
# The board runs GomSpace Linux 4.1.4, built on Yocto 4.1 "langdale", which
# ships glibc 2.36. Debian 12 ships the same glibc, so binaries linked here
# only use C library symbol versions the board provides.
FROM debian:bookworm@sha256:2c037a04925515fdd6ea85ea14a682d0e79931f5e9f5d07b6dbfc6ba12f9e858

RUN dpkg --add-architecture armhf \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        crossbuild-essential-armhf \
        git \
        libgcrypt20-dev:armhf \
        libsocketcan-dev:armhf \
        python3 \
    && rm -rf /var/lib/apt/lists/*
