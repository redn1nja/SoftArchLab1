#include "crow.h"
#include "hazelcast/client/hazelcast_client.h"
using namespace std::literals;
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
    auto address = argc > 1 ? argv[1] : "localhost"s;
    hz::client_config config;
    config.set_cluster_name("q");
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(address, 5701)));
    auto client = hazelcast::new_client(std::move(config)).get();
    auto queue = client.get_queue("queue").get();
    auto port = argc > 2 ? std::stoi(argv[2]) : 5000;
    crow::SimpleApp app;
    std::vector<std::string> messages;
    auto handle = std::thread(queue_handler<decltype(queue)>, queue, &messages);
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([&messages, &port]() {
        auto response = "message:"s + std::to_string(port);
        response = std::accumulate(messages.begin(), messages.end(), response, [](const auto& response, auto& message){
            return response + message + "\n";
        });
        return crow::response(200,response);
    });
    app.bindaddr("0.0.0.0").port(port).loglevel(crow::LogLevel::INFO).multithreaded().run();
    handle.join(); // so pointer won't be invalidated
}