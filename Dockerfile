ARG TARGET=linux-x64
FROM dockcross/${TARGET}:latest
RUN apt-get update && apt-get install -y python3-dev 
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 1

WORKDIR /build
