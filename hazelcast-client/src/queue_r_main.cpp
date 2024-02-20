#include <iostream>
#include <hazelcast/client/hazelcast.h>

namespace hz = hazelcast::client;
int main(int argc, char **argv){
    hz::client_config config;
    config.set_cluster_name("hello-world");
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(argv[1], 5701)));
    auto client = hazelcast::new_client(std::move(config)).get();
    auto queue = client.get_queue("queue").get();
    while(true){
        auto item = queue->take<int>().get();
        if (item){
            std::cout<<"Read: "<<item.value()<<std::endl;
            if(item.value() == -1){
                queue->put(-1).get();
                break;
            }
        }
    }
    client.shutdown().get();
    return 0;
}