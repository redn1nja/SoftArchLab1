#include "crow.h"
#include "hazelcast/client/hazelcast_client.h"
#include "ppconsul/kv.h"
#include "ppconsul/agent.h"

using namespace std::literals;
using ppconsul::Consul;
namespace cna = ppconsul::agent;
namespace cnk = ppconsul::kv;
namespace hz = hazelcast::client;

template <typename Q>
[[noreturn]] void queue_handler(Q&& queue, std::vector<std::string>* messages){
    while (true) {
        auto message = queue->template take<std::string>().get();
        std::cout << "Received: " << message.value() << std::endl;
        messages->push_back(message.value());
    }
}

int main(int argc, char* argv[]) {
    std::string hostname = getenv("HOSTNAME");
    std::string port = argv[1];
    std::string address = std::string(hostname) + ":"s + port;
    hz::client_config config;
    Consul consul{"http://consul:8500"};
    cna::Agent agent{consul};
    agent.registerService(cna::kw::name = "message"s,
                          cna::kw::address = std::string(hostname),
                          cna::kw::id = address,
                          cna::kw::port = std::stoi(port),
                          cna::kw::tags = {"http"});
    cnk::Kv kv{consul};

    auto queue_cluster = crow::json::load(kv.get("hazelcast/cluster/queue_cluster", "{}"));

    auto cluster_name = queue_cluster["name"].s();
    auto hz_address = queue_cluster["members"]["name"].s();
    auto hz_port = queue_cluster["members"]["port"].i();

    auto queue_name = kv.get("hazelcast/queue", "");

    config.set_cluster_name(cluster_name);
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(hz_address, hz_port)));
    auto client = hazelcast::new_client(std::move(config)).get();
    auto queue = client.get_queue(queue_name).get();
    crow::SimpleApp app;
    std::vector<std::string> messages;
    auto handle = std::thread(queue_handler<decltype(queue)>, queue, &messages);
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([&messages, &port]() {
        std::string response = std::accumulate(messages.begin(), messages.end(), ""s, [](const auto& response, auto& message){
            return response + message + "\n";
        });
        return crow::response(200,response);
    });
    app.bindaddr("0.0.0.0").port(std::stoi(port)).loglevel(crow::LogLevel::INFO).multithreaded().run();
    handle.join(); // so pointer won't be invalidated
}