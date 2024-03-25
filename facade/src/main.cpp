#include "crow.h"
#include "hazelcast/client/hazelcast_client.h"
#include "cpr/cpr.h"
#include <random>
#include "boost/uuid/uuid.hpp"

using namespace std::literals;
namespace hz = hazelcast::client;

struct rd {
    std::random_device rn;
    std::mt19937 gen;
    std::uniform_int_distribution<size_t> dis_l;
    std::uniform_int_distribution<size_t> dis_m;
    rd(size_t l, size_t m) : rn{}, gen{rn()}, dis_l(0, l), dis_m(0, m){}
    std::pair<size_t, size_t> operator()(){
        return {dis_l(gen), dis_m(gen)};
    }
};

int main(int argc, char* argv[]){
    boost::uuids::random_generator generator;
    std::cout << argv[1] << " " << argv[2] << std::endl;
    std::string address = argv[1];
    int port = std::stoi(argv[2]);
    int loggerc = std::stoi(argv[3]);
    std::vector<std::string> loggers;
    std::vector<std::string> messagers;
    loggers.reserve(loggerc);
    for (int i = 0; i < loggerc; i++){
        loggers.emplace_back(argv[4 + i]);
        std::cout << "logger: " << loggers[i] << std::endl;
    }
    for (int i = 4+loggerc; i < argc; ++i) {
        messagers.emplace_back(argv[i]);
        std::cout << "messager: " << messagers[i-loggerc-4] << std::endl;
    }

    crow::SimpleApp app;
    hz::client_config config;
    config.set_cluster_name("q");
    config.set_network_config(hz::config::client_network_config().add_address(hz::address(address, 5701)));
    auto client = hazelcast::new_client(std::move(config)).get();
    auto queue = client.get_queue("queue").get();
    auto randomizer = rd(loggers.size()-1, messagers.size()-1);
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)([&randomizer, &loggers, &messagers]() {
        auto [l, m] = randomizer();
        auto logg = cpr::Get(cpr::Url{loggers[l]});
        auto mess = cpr::Get(cpr::Url{messagers[m]});
        std::string response = "logger: " + loggers[l] + "\n" + logg.text + "\n" + "messager: " + messagers[m] + "\n" + mess.text + "\n";
        return crow::response(200, response);
    });
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::POST)([&queue, &randomizer, &generator, &loggers](const crow::request& req) {
        auto [l, _] = randomizer();
        auto message = crow::json::load(req.body);
        auto uuid = generator();
        auto str = to_string(uuid);
        auto msg = str + " " + message["msg"].s().operator std::string();
        CROW_LOG_INFO << "POST request received by logging service writing values " << msg << "\n";
        queue->offer(msg).get();
        auto fmt = R"({"uuid": "%s", "msg": "%s"})";
        char body[1024];
        std::sprintf(body, fmt, str.c_str(), message["msg"].s().operator std::string().c_str());
        auto r = cpr::Post(cpr::Url{loggers[l]},
                           cpr::Body{body});
        return crow::response(200, ""s);
    });
    app.bindaddr("0.0.0.0").port(port).loglevel(crow::LogLevel::INFO).multithreaded().run();
}