#include <iostream>
#include <hazelcast/client/hazelcast.h>

namespace hz = hazelcast::client;
int main(int argc, char **argv){
    hz::client_config config;
    config.set_cluster_name("hello-world");
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(argv[1], 5701)));
    auto client = hazelcast::new_client(std::move(config)).get();
    auto map = client.get_map("map").get();
    map->put_if_absent("key", 0);
    for (int i = 0 ; i < 10000; ++i) {
#ifdef NOLOCKS
        auto value = map->get<std::string, int>("key").get().get();
        map->put("key", value + 1).get();

#elif LOCKS_PESSIMISTIC_ENABLED
        map->lock("key").get();
        auto value = map->get<std::string, int>("key").get().get();
        map->put("key", value + 1).get();
        map->unlock("key").get();
#elif LOCKS_OPTIMISTIC_ENABLED
    while(true) {
        auto old = map->get<std::string, int>("key").get().get();
        auto new_value = old + 1;
        if (map->replace("key", old, new_value).get()) {
            break;
        }
    }
#endif
    }
    client.shutdown().get();
    return 0;
}