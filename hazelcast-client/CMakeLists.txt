cmake_minimum_required(VERSION 3.22)
set(PROJECT_NAME hazel_client)

project(${PROJECT_NAME} C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(hazelcast-cpp-client CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE hazelcast-cpp-client::hazelcast-cpp-client)

add_executable(locks_1 src/locks_main.cpp)
target_link_libraries(locks_1 PRIVATE hazelcast-cpp-client::hazelcast-cpp-client)
target_compile_definitions(locks_1 PRIVATE NOLOCKS)

add_executable(locks_2 src/locks_main.cpp)
target_link_libraries(locks_2 PRIVATE hazelcast-cpp-client::hazelcast-cpp-client)
target_compile_definitions(locks_2 PRIVATE LOCKS_PESSIMISTIC_ENABLED)

add_executable(locks_3  src/locks_main.cpp)
target_link_libraries(locks_3 hazelcast-cpp-client::hazelcast-cpp-client)
target_compile_definitions(locks_3 PRIVATE LOCKS_OPTIMISTIC_ENABLED)

add_executable(queue_r src/queue_r_main.cpp)
target_link_libraries(queue_r PRIVATE hazelcast-cpp-client::hazelcast-cpp-client)

add_executable(queue_w src/queue_w_main.cpp)
target_link_libraries(queue_w PRIVATE hazelcast-cpp-client::hazelcast-cpp-client)