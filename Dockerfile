# Download base image ubuntu 20.04
FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive

# Update sources
RUN apt-get update && apt-get upgrade -y

# Install zlib, libssl, g++, cmake, git
RUN apt-get install build-essential -y libssl-dev zlib1g-dev cmake git

# Install python 3
RUN apt-get install -y python3-dev && \
    update-alternatives --install /usr/bin/python python /usr/bin/python3 1

# Install bison & flex
RUN apt-get install -y bison flex

WORKDIR /wasmati
COPY ./ /wasmati

# Prepare Cmake
RUN mkdir -p build && cd build/ && cmake ../

# Build wasmati 
RUN cmake --build /wasmati/build --target wasmati

# Build wasmati-query
RUN cmake --build /wasmati/build --target wasmati-query
