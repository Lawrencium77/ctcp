FROM ubuntu:latest
RUN apt-get update && apt-get install -y build-essential && apt-get install -y dnsutils

WORKDIR /app
COPY . /app

RUN make