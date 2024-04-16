CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
project(facade)

set(CMAKE_CXX_STANDARD 20)

add_executable(facade src/main.cpp)
find_package(Crow)
find_package(hazelcast-cpp-client CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(ppconsul REQUIRED)
find_package(cpr REQUIRED)

target_link_libraries(facade PUBLIC Crow::Crow)
target_link_libraries(facade PUBLIC hazelcast-cpp-client::hazelcast-cpp-client)
target_include_directories(facade PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(facade PUBLIC CURL::libcurl)
target_link_libraries(facade PUBLIC ppconsul)
target_link_libraries(facade PUBLIC cpr::cpr)

