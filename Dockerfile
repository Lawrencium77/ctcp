FROM ubuntu:latest
RUN apt-get update && apt-get install -y \
    build-essential \
    dnsutils \
    iproute2 \
    gdb \
    bear

WORKDIR /app
COPY . /app