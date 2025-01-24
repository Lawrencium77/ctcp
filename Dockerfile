FROM ubuntu:latest
RUN apt-get update && apt-get install -y \
    clangd \
    clang-tidy \
    clang-format \
    cmake \
    build-essential \
    dnsutils \
    iproute2 \
    gdb \
    bear

WORKDIR /app
COPY . /app