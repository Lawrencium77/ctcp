FROM ubuntu:latest
RUN apt-get update && apt-get install -y \
    build-essential \
    dnsutils \
    iproute2 \
    gdb

WORKDIR /app
COPY . /app