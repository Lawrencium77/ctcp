FROM ubuntu:latest
RUN apt-get update && apt-get install -y build-essential && apt-get install -y dnsutils iproute2

WORKDIR /app
COPY . /app