#include "crow.h"
#include "hazelcast/client/hazelcast_client.h"
#include "cpr/cpr.h"
#include <random>
#include "ppconsul/kv.h"
#include "ppconsul/agent.h"
#include "ppconsul/health.h"


using namespace std::literals;
namespace hz = hazelcast::client;
using ppconsul::Consul;
namespace cna = ppconsul::agent;
namespace cnk = ppconsul::kv;
namespace cnh = ppconsul::health;

struct Generator{
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<std::size_t> dis;
    explicit Generator(std::size_t size) : gen(rd()), dis(0, size){}
    std::size_t operator()() {
        return dis(gen);
    }
};

//constexpr std::string_view hostname = "facade";
int main(int argc, char* argv[]){
    crow::SimpleApp app;
    std::string hostname = getenv("HOSTNAME");
    hz::client_config config;
    boost::uuids::random_generator generator;

    std::string port = argv[1];
    std::this_thread::sleep_for(10s);
    std::string address = std::string(hostname) + ":"s + port;
    Consul consul{"http://consul:8500"};

    cna::Agent agent{consul};
    agent.registerService(cna::kw::name = "facade"s,
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

    cnh::Health health{consul};
    auto loggers = health.service("logger");
    auto messagers = health.service("message");

    std::vector<std::string> logger_services;
    std::vector<std::string> message_services;

    std::transform(loggers.begin(), loggers.end(), std::back_inserter(logger_services), [](const auto& element){
        auto service = std::get<1>(element);
        return "http://"s + service.address + ":"s + std::to_string(service.port);
    });

    std::transform(messagers.begin(), messagers.end(), std::back_inserter(message_services), [](const auto& element){
        auto service = std::get<1>(element);
        return "http://"s + service.address + ":"s + std::to_string(service.port);
    });
    Generator logger_generator(logger_services.size()-1);
    Generator message_generator(message_services.size()-1);

    config.set_cluster_name(cluster_name);
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(hz_address, hz_port)));

    auto client = hazelcast::new_client(std::move(config)).get();
    auto queue = client.get_queue(queue_name).get();

    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([&logger_services, &message_services, &logger_generator, &message_generator]() {
        std::string response = "Logger service: ";
        auto service = logger_services[logger_generator()];
        auto logger = cpr::Get(cpr::Url{service});
        response += logger.text;
        response += "\nMessage service:\n";
        service = message_services[message_generator()];
        auto message = cpr::Get(cpr::Url{service});
        response += message.text;
        response += "\n";
        return crow::response(200, response);
    });

    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::POST)([&generator, &queue, &logger_services, &logger_generator](const crow::request& req) {
        auto message = crow::json::load(req.body);
        auto uuid = generator();
        auto str = to_string(uuid);
        auto msg = str + " " + message["msg"].s().operator std::string();
        auto service = logger_services[logger_generator()];
        CROW_LOG_INFO << "POST request received by logger service "<<service<<" writing values " << msg << "\n";
        queue->offer(msg).get();
        auto fmt = R"({"uuid": "%s", "msg": "%s"})";
        char body[1024];
        std::sprintf(body, fmt, str.c_str(), message["msg"].s().operator std::string().c_str());
        auto response = cpr::Post(cpr::Url{service}, cpr::Body{body}, cpr::Header{{"Content-Type", "application/json"}});
        return crow::response(200, ""s);
    });
    app.bindaddr("0.0.0.0").port(std::stoi(port)).loglevel(crow::LogLevel::INFO).multithreaded().run();
}