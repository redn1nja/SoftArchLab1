# syntax=docker/dockerfile:1
FROM ubuntu:20.04 as base
ARG DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y curl g++ cmake make pkg-config libboost-all-dev git libssl-dev libmbedtls-dev; \
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
CMD ["sh", "-c", "/usr/src/facade $ADDR $PORT 3 http://logger-1:3001 http://logger-2:5000 http://logger-3:7000 http://message-1:5001 http://message-2:5002 http://message-3:5003"]