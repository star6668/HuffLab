cmake_minimum_required(VERSION 3.20)
project(HuffLabConsumer LANGUAGES CXX)

find_package(HuffLab 0.2 CONFIG REQUIRED)

add_executable(consumer_example main.cpp)
target_link_libraries(consumer_example PRIVATE HuffLab::hufflab)

enable_testing()
add_test(NAME consume_installed_hufflab COMMAND consumer_example)
