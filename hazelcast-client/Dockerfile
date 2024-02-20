FROM ubuntu:22.04 AS base
RUN apt-get update;

FROM base AS builder
RUN  apt-get install -y g++ curl cmake libzmq3-dev libboost-all-dev; \
        mkdir -p /usr/src/; \
        cd /usr/src/; \
        curl -L https://github.com/hazelcast/hazelcast-cpp-client/archive/refs/tags/v5.3.0.tar.gz | tar xzf -; \
        cd hazelcast-cpp-client-5.3.0; \
        cmake -DCMAKE_BUILD_TYPE=Release .; make; make install

COPY . /usr/src/hazelcast-client

RUN set -ex; \
        cd /usr/src/hazelcast-client; \
        cmake -DCMAKE_BUILD_TYPE=Release .; make


FROM builder as final_client
COPY --from=builder /usr/src/hazelcast-client/hazel_client /usr/bin/client
CMD ["/usr/bin/client", "hazelcast1"] 

FROM builder as final_locks1
COPY --from=builder /usr/src/hazelcast-client/locks_1 /usr/bin/locks1
CMD ["/usr/bin/locks1", "hazelcast1"]

FROM builder as final_locks2
COPY --from=builder /usr/src/hazelcast-client/locks_2 /usr/bin/locks2
CMD ["/usr/bin/locks2", "hazelcast1"]

FROM builder as final_locks3
COPY --from=builder /usr/src/hazelcast-client/locks_3 /usr/bin/locks3
CMD ["/usr/bin/locks3", "hazelcast1"]

FROM builder as queue_r
COPY --from=builder /usr/src/hazelcast-client/queue_r /usr/bin/queue_r
CMD ["/usr/bin/queue_r", "hazelcast1"]

FROM builder as queue_w
COPY --from=builder /usr/src/hazelcast-client/queue_w /usr/bin/queue_w
CMD ["/usr/bin/queue_w", "hazelcast1"]