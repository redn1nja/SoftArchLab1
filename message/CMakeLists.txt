CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
project(message)

set(CMAKE_CXX_STANDARD 20)

add_executable(message src/main.cpp)
find_package(Crow)
find_package(hazelcast-cpp-client CONFIG REQUIRED)

target_link_libraries(message PUBLIC Crow::Crow)
target_link_libraries(message PUBLIC hazelcast-cpp-client::hazelcast-cpp-client)