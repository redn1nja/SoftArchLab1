# syntax=docker/dockerfile:1
FROM ubuntu:20.04 as base
ARG DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y curl g++ cmake make pkg-config libboost-all-dev git ; \
    mkdir /usr/src && cd /usr/src; \
    curl -L https://github.com/CrowCpp/Crow/releases/download/v1.0%2B5/crow-v1.0+5.deb -o crow.deb; \
    dpkg -i crow.deb; \
    curl -L https://github.com/hazelcast/hazelcast-cpp-client/archive/refs/tags/v5.3.0.tar.gz | tar xzf -; \
            cd hazelcast-cpp-client-5.3.0; \
            cmake -DCMAKE_BUILD_TYPE=Release .; make; make install

COPY . /usr/src/

FROM base as build
WORKDIR /usr/src/
RUN cmake -DCMAKE_BUILD_TYPE=Release . && make

FROM build as final
EXPOSE $PORT
CMD ["sh", "-c", "/usr/src/message $ADDR $PORT"]