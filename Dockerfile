# syntax=docker/dockerfile:1
FROM ubuntu:20.04 as base
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y curl g++ cmake make pkg-config libboost-all-dev git libcurl4-openssl-dev libssl-dev libmbedtls-dev;  \
    mkdir -p /usr/src && cd /usr/src;
FROM base as crow
RUN curl -L https://github.com/CrowCpp/Crow/releases/download/v1.0%2B5/crow-v1.0+5.deb -o crow.deb; \
    dpkg -i crow.deb;
FROM crow as hazelcast
RUN curl -L https://github.com/hazelcast/hazelcast-cpp-client/archive/refs/tags/v5.3.0.tar.gz | tar xzf -; \
            cd hazelcast-cpp-client-5.3.0; \
            cmake -DCMAKE_BUILD_TYPE=Release .; make; make install; cd ..
FROM hazelcast as ppconsul
RUN curl -L https://github.com/oliora/ppconsul/archive/refs/tags/v0.2.3.tar.gz | tar xzf -; \
            cd ppconsul-0.2.3; \
            cmake -DCMAKE_BUILD_TYPE=Release .; make ; make install; cd ..
FROM ppconsul as common
RUN curl -L https://github.com/libcpr/cpr/archive/refs/tags/1.10.5.tar.gz | tar xzf -; \
            cd cpr-1.10.5; \
            cmake -DCPR_USE_SYSTEM_CURL=ON -DCMAKE_BUILD_TYPE=Release .; make; make install
