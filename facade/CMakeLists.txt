CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
project(facade)

set(CMAKE_CXX_STANDARD 20)

add_executable(facade src/main.cpp)
find_package(Crow)
find_package(hazelcast-cpp-client CONFIG REQUIRED)

target_link_libraries(facade PUBLIC Crow::Crow)
target_link_libraries(facade PUBLIC hazelcast-cpp-client::hazelcast-cpp-client)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8) # Replace with your desired git commit from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)
target_link_libraries(facade PRIVATE cpr::cpr)
