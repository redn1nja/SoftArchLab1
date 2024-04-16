#include "crow.h"
#include "hazelcast/client/hazelcast_client.h"
#include "ppconsul/health.h"
#include "ppconsul/agent.h"
#include "ppconsul/kv.h"

#include <chrono>

using namespace std::literals;
namespace hz = hazelcast::client;
using ppconsul::Consul;
namespace cna = ppconsul::agent;
namespace cnk = ppconsul::kv;
namespace cnh = ppconsul::health;

int main(int argc, char* argv[])
{
    crow::SimpleApp app;
    hz::client_config config;
    std::string hostname = getenv("HOSTNAME");
    std::string port = argv[1];
    std::string address = std::string(hostname) + ":"s + port;
    Consul consul{"http://consul:8500"};

    cna::Agent agent{consul};
    agent.registerService(cna::kw::name = "logger"s,
                          cna::kw::address = std::string(hostname),
                          cna::kw::id = address,
                          cna::kw::port = std::stoi(port),
                          cna::kw::tags = {"http"});


    cnk::Kv kv{consul};
    auto cluster_info = crow::json::load(kv.get("hazelcast/cluster/map_cluster", "{}"));
    auto hz_address = cluster_info["members"]["name"].s();
    auto hz_port = cluster_info["members"]["port"].i();
    auto map_name = kv.get("hazelcast/map", "map1");

    config.set_cluster_name(cluster_info["name"].s());
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(hz_address, hz_port)));

    auto client = hazelcast::new_client(std::move(config)).get();
    auto map = client.get_map(map_name).get();

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
    app.bindaddr("0.0.0.0").port(std::stoi(port)).loglevel(crow::LogLevel::INFO).multithreaded().run();
}