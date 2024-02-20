#include <iostream>
#include <hazelcast/client/hazelcast.h>

namespace hz = hazelcast::client;
int main(int argc, char **argv){
    hz::client_config config;
    config.set_cluster_name("hello-world");
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(argv[1], 5701)));
    auto client = hazelcast::new_client(std::move(config)).get();
    auto map = client.get_map("map").get();
    for (int i = 0; i < 1000; i++) {
        map->put<int ,std::string>(i, "Hello World!").get();
    }
    client.shutdown().get();
    return 0;
}
