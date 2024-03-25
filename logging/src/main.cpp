#include "crow.h"
#include "hazelcast/client/hazelcast_client.h"
using namespace std::literals;
namespace hz = hazelcast::client;
int main(int argc, char* argv[])
{
    std::cout << argv[1] << " " << argv[2] << std::endl;
    std::string address = argv[1];
    int port = std::stoi(argv[2]);
    crow::SimpleApp app;
    hz::client_config config;
    config.set_cluster_name("hello-world");
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(address, 5701)));
    auto client = hazelcast::new_client(std::move(config)).get();
    auto map = client.get_map("map").get();

    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([&map, &address](){
        auto val = map->entry_set<std::string, std::string>().get();
        std::string result;
        CROW_LOG_INFO << "GET request received by logging service connected to hazelcast node " << address << "\n";
        result = std::accumulate(val.begin(), val.end(), ""s, [](const std::string &result, auto &entry){
            return (result + entry.second + "\n");
        });
        return crow::response(200, result);
    });

    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::POST)([&map, &address](const crow::request &req){
        std::cout<<req.body<<std::endl;
        auto x = crow::json::load(req.body);
        if (!x && !x.has("uuid") && !x.has("msg"))
            return crow::response(400);
        CROW_LOG_INFO << "POST request received by logging service connected to hazelcast node " << address << " writing values" \
                                                                        << x["uuid"].s() << " " << x["msg"].s() << "\n";
        map->put<std::string, std::string>(
                x["uuid"].s().operator std::string(),
                x["msg"].s().operator std::string())
                .get();
        return crow::response(200);
    });
    app.bindaddr("0.0.0.0").port(port).loglevel(crow::LogLevel::INFO).multithreaded().run();
}